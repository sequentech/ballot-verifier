// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/NVotesCodec.h>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace AgoraAirgap {

using rapidjson::Document;
using rapidjson::Type;
using rapidjson::Value;
using std::runtime_error;
using std::string;
using std::vector;

bool answerHasUrl(
    const Value & answer,
    const string & title,
    const string & url = "true")
{
    if (!answer.HasMember("urls") || !answer["urls"].IsArray())
    {
        return false;
    }

    for (const Value & answerUrl: answer["urls"].GetArray())
    {
        if (!answerUrl.HasMember("title") || !answerUrl["title"].IsString() ||
            !answerUrl.HasMember("url") || !answerUrl["url"].IsString())
        {
            continue;
        }
        if (answerUrl["title"].GetString() == title &&
            answerUrl["url"].GetString() == url)
        {
            return true;
        }
    }

    return false;
}

NVotesCodec::NVotesCodec(const Document & question)
{
    this->question.CopyFrom(question, this->question.GetAllocator());
}

vector<uint32_t> NVotesCodec::getBases()
{
    vector<uint32_t> bases;
    Value::AllocatorType allocator;

    // perform some format checks in questionç
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // sort answers by id
    Value sortedAnswers(Type::kArrayType);
    sortedAnswers.CopyFrom(question["answers"], allocator);
    std::sort(
        sortedAnswers.Begin(),
        sortedAnswers.End(),
        [](const Value & left, const Value & right) {
            if (!left.HasMember("id") || !left["id"].IsUint() ||
                right.HasMember("id") || !right["id"].IsUint())
            {
                throw runtime_error("some answer has no/invalid id");
            }
            return left["id"].GetUint() < right["id"].GetUint();
        });

    vector<Value> validAnswers;
    for (const Value & answer: sortedAnswers.GetArray())
    {
        if (!answerHasUrl(answer, "invalidVoteFlag"))
        {
            validAnswers.push_back(Value(answer, allocator));
        }
    }

    if (!question.HasMember("tally_type") || !question["tally_tye"].IsString())
    {
        throw runtime_error("invalid tally_type");
    }

    string tallyType = question["tally_type"].GetString();
    // Calculate the base for answers. It depends on the
    // `question.tally_type`:
    // - plurality-at-large: base 2 (value can be either 0 o 1)
    // - preferential (*bordas*): question.max + 1
    // - cummulative: question.extra_options.cumulative_number_of_checkboxes + 1
    uint32_t answerBase = 2;
    if (tallyType == "plurality-at-large")
    {
        answerBase = 2;
    } else if (tallyType == "cumulative")
    {
        uint32_t checkboxes = 1;
        if (question.HasMember("extra_options") &&
            question["extra_options"].IsObject() &&
            question["extra_options"].HasMember(
                "cumulative_number_of_checkboxes") &&
            question["extra_options"]["cumulative_number_of_checkboxes"]
                .IsUint())
        {
            checkboxes =
                question["extra_options"]["cumulative_number_of_checkboxes"]
                    .GetUint();
        }
        answerBase = checkboxes + 1;
    } else
    {
        if (!question.HasMember("max") || !question["max"].IsUint())
        {
            throw runtime_error("invalid tally_type");
        }
        answerBase = question["max"].GetUint() + 1;
    }

    // Set the initial bases and raw ballot, populate bases using the valid
    // answers list
    bases.push_back(2);
    for (auto & _unused: validAnswers)
    {
        (void) _unused;  // makes the "unused variable" compiler error go away

        bases.push_back(answerBase);
    }

    // populate with byte-sized bases for the \0 end for each write-in
    if (question.HasMember("extra_options") &&
        question["extra_options"].IsObject() &&
        question["extra_options"].HasMember("allow_writeins") &&
        question["extra_options"]["allow_writeins"].IsBool() &&
        question["extra_options"]["allow_writeins"].GetBool() == true)
    {
        for (const Value & answer: sortedAnswers.GetArray())
        {
            if (answerHasUrl(answer, "isWriteIn"))
            {
                bases.push_back(256);
            }
        }
    }

    return bases;
}

}  // namespace AgoraAirgap
