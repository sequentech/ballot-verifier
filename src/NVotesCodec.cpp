// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/MixedRadix.h>
#include <agora-airgap/NVotesCodec.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace AgoraAirgap {

using rapidjson::Document;
using rapidjson::Type;
using rapidjson::Value;
using std::runtime_error;
using std::string;
using std::unique_ptr;
using std::vector;

string stringify(const Value & value)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

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

Value sortAnswers(
    const Value & answers,
    const char * fieldName,
    Value::AllocatorType & allocator)
{
    Value sortedAnswers(Type::kArrayType);
    sortedAnswers.CopyFrom(answers, allocator);
    std::sort(
        sortedAnswers.Begin(),
        sortedAnswers.End(),
        [fieldName](const Value & left, const Value & right) {
            if (!left.HasMember(fieldName) || !left[fieldName].IsUint() ||
                !right.HasMember(fieldName) || !right[fieldName].IsUint())
            {
                throw runtime_error("some answer has no/invalid id");
            }
            return left[fieldName].GetUint() < right[fieldName].GetUint();
        });
    return sortedAnswers;
}

NVotesCodec::NVotesCodec(const Document & question)
{
    this->question.CopyFrom(question, this->question.GetAllocator());
}

vector<uint32_t> NVotesCodec::getBases() const
{
    vector<uint32_t> bases;
    Value::AllocatorType allocator;

    // perform some format checks in questionç
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // sort answers by id
    Value sortedAnswers = sortAnswers(question["answers"], "id", allocator);

    vector<rapidjson::Value> validAnswers;
    for (const Value & answer: sortedAnswers.GetArray())
    {
        if (!answerHasUrl(answer, "invalidVoteFlag"))
        {
            validAnswers.push_back(rapidjson::Value(answer, allocator));
        }
    }

    if (!question.HasMember("tally_type") || !question["tally_type"].IsString())
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
            throw runtime_error("invalid max");
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

RawBallot NVotesCodec::encodeRawBallot() const
{
    Value::AllocatorType allocator;

    // sort answers by id
    Value sortedAnswers = sortAnswers(question["answers"], "id", allocator);

    // perform some format checks in question
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // Separate the answers between:
    // - Invalid vote answer (if any)
    // - Write-ins (if any)
    // - Valid answers (normal answers + write-ins if any)
    unique_ptr<Value> invalidVoteAnswer(nullptr);
    uint32_t invalidVoteFlag = 0;
    vector<Value> writeInAnswers;
    vector<Value> validAnswers;
    for (const Value & answer: sortedAnswers.GetArray())
    {
        if (answerHasUrl(answer, "invalidVoteFlag"))
        {
            invalidVoteAnswer = unique_ptr<Value>(new Value(answer, allocator));
            if (invalidVoteAnswer->HasMember("selected") &&
                (*invalidVoteAnswer)["selected"].IsInt() &&
                (*invalidVoteAnswer)["selected"].GetInt() > -1)
            {
                invalidVoteFlag = 1;
            }
        } else
        {
            validAnswers.push_back(Value(answer, allocator));
        }
        if (answerHasUrl(answer, "isWriteIn"))
        {
            writeInAnswers.push_back(Value(answer, allocator));
        }
    }

    // Set the initial bases and raw ballot. We will populate the rest next
    vector<uint32_t> bases = getBases();
    vector<uint32_t> choices;
    choices.push_back(invalidVoteFlag);

    // populate raw_ballot and bases using the valid answers list
    if (!question.HasMember("tally_type") || !question["tally_type"].IsString())
    {
        throw runtime_error("invalid tally_type");
    }
    string tallyType = question["tally_type"].GetString();
    for (const Value & answer: validAnswers)
    {
        if (tallyType == "plurality-at-large")
        {
            // We just flag if the candidate was selected or not with 1 for
            // selected and 0 otherwise
            uint32_t answerValue =
                (answer.HasMember("selected") && answer["selected"].IsInt() &&
                 answer["selected"].GetInt() > -1)
                    ? 1
                    : 0;
            choices.push_back(answerValue);
        } else
        {
            // we add 1 because the counting starts with 1, as zero means this
            // answer was not voted / ranked
            uint32_t answerValue =
                (answer.HasMember("selected") && answer["selected"].IsInt())
                    ? answer["selected"].GetInt() + 1
                    : 0;
            choices.push_back(answerValue);
        }
    }
    // Populate the bases and the raw_ballot values with the write-ins
    // if there's any. We will through each write-in (if any), and then
    // encode the write-in answer.text string with UTF-8 and use for
    // each byte a specific value with base 256 and end each write-in
    // with a \0 byte. Note that even write-ins.
    if (question.HasMember("extra_options") &&
        question["extra_options"].IsObject() &&
        question["extra_options"].HasMember("allow_writeins") &&
        question["extra_options"]["allow_writeins"].IsBool() &&
        question["extra_options"]["allow_writeins"].GetBool() == true)
    {
        for (const Value & answer: writeInAnswers)
        {
            if (!answer.HasMember("text") || !answer["text"].IsString())
            {
                throw runtime_error("invalid answer text");
            }
            string answerText = answer["text"].GetString();
            if (answerText.length() == 0)
            {
                // we don't do a bases.push_back(256) as this is done in
                // getBases() to end it with a zero
                choices.push_back(0);
                continue;
            }

            const char * encodedText = answerText.c_str();
            for (size_t index = 0; index < strlen(encodedText); index++)
            {
                const char & byte = encodedText[index];
                bases.push_back(256);
                choices.push_back(static_cast<uint8_t>(byte));
            }

            // End it with a zero. we don't do a bases.push_back(256) as this is
            // done in getBases()
            choices.push_back(0);
        }
    }
    return RawBallot{/* bases = */ bases,
                     /* choices = */ choices};
}

mpz_class NVotesCodec::encodeToInt(const RawBallot & rawBallot) const
{
    return MixedRadix::encode(rawBallot.choices, rawBallot.bases);
}

}  // namespace AgoraAirgap
