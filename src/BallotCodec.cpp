// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <ballot-verifier/BallotCodec.h>
#include <ballot-verifier/MixedRadix.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace BallotVerifier {

using rapidjson::Document;
using rapidjson::Type;
using rapidjson::Value;
using std::endl;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

bool answerHasUrl(
    const Value & answer,
    const string & title,
    const string & url)
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

/**
 * @returns a deep copy of an answer array sorted by fieldName
 */
Value cloneSortedAnswers(
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

vector<Value *> sortedAnswersVector(Value & answers, const char * fieldName)
{
    vector<Value *> sortedAnswers;
    std::transform(
        answers.GetArray().Begin(),
        answers.GetArray().End(),
        std::back_inserter(sortedAnswers),
        [](Value & answer) { return &answer; });

    std::sort(
        sortedAnswers.begin(),
        sortedAnswers.end(),
        [fieldName](const Value * left, const Value * right) {
            if (!right->HasMember(fieldName) || !(*right)[fieldName].IsInt())
            {
                stringstream error;
                error << "right answer has no/invalid field" << fieldName
                      << ": " << stringify(*right);
                throw runtime_error(error.str());
            }
            if (!left->HasMember(fieldName) || !(*left)[fieldName].IsInt())
            {
                stringstream error;
                error << "left answer has no/invalid field " << fieldName
                      << ": " << stringify(*left);
                throw runtime_error(error.str());
            }
            return (*left)[fieldName].GetInt() < (*right)[fieldName].GetInt();
        });
    return sortedAnswers;
}

BallotCodec::BallotCodec(const Document & question)
{
    this->question.CopyFrom(question, this->question.GetAllocator());
}

vector<uint32_t> BallotCodec::getBases() const
{
    vector<uint32_t> bases;
    Value::AllocatorType allocator;

    // perform some format checks in questionç
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // sort answers by id
    Value sortedAnswers =
        cloneSortedAnswers(question["answers"], "id", allocator);

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

RawBallot BallotCodec::encodeRawBallot() const
{
    Value::AllocatorType allocator;

    // sort answers by id
    Value sortedAnswers =
        cloneSortedAnswers(question["answers"], "id", allocator);

    // perform some format checks in question
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // Separate the answers between:
    // - Invalid vote answer (if any)
    // - Write-ins (if any)
    // - Valid answers (normal answers + write-ins if any)
    Value * invalidVoteAnswer = nullptr;
    uint32_t invalidVoteFlag = 0;
    vector<Value> writeInAnswers;
    vector<Value> validAnswers;
    for (const Value & answer: sortedAnswers.GetArray())
    {
        if (answerHasUrl(answer, "invalidVoteFlag"))
        {
            invalidVoteAnswer = new Value(answer, allocator);
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

mpz_class BallotCodec::encodeToInt(const RawBallot & rawBallot) const
{
    return MixedRadix::encode(rawBallot.choices, rawBallot.bases);
}

RawBallot BallotCodec::decodeFromInt(const mpz_class & intBallot) const
{
    vector<uint32_t> bases = getBases();
    size_t basesSize = bases.size();
    uint32_t lastBase = 256;
    std::vector<uint32_t> choices = MixedRadix::decode(
        /* baseList = */ bases,
        /* encodedValue = */ intBallot,
        /* lastBase = */ &lastBase);

    // apply changes required for the write-ins
    if (question.HasMember("extra_options") &&
        question["extra_options"].IsObject() &&
        question["extra_options"].HasMember("allow_writeins") &&
        question["extra_options"]["allow_writeins"].IsBool() &&
        question["extra_options"]["allow_writeins"].GetBool() == true)
    {
        // make the number of bases equal to the number of choices
        size_t index = basesSize + 1;
        while (index <= choices.size())
        {
            bases.push_back(256);
            index++;
        }

        // count number of write-ins
        size_t numWriteInAnswers = std::count_if(
            question["answers"].GetArray().Begin(),
            question["answers"].GetArray().End(),
            [](const Value & answer) {
                return answerHasUrl(answer, "isWriteIn");
            });

        // count number of empty writeIn choices
        size_t numWriteInStrings = 0;
        size_t writeInsTextStartIndex = basesSize - numWriteInAnswers;
        size_t index2 = writeInsTextStartIndex;
        while (index2 < choices.size())
        {
            if (choices[index2] == 0)
            {
                numWriteInStrings++;
            }
            index2++;
        }

        // Each non empty write-in needs another 0
        size_t index3 = 0;
        size_t numNonEmptyWriteIns = numWriteInAnswers - numWriteInStrings;
        while (index3 < numNonEmptyWriteIns)
        {
            bases.push_back(256);
            choices.push_back(0);
            index3++;
        }
    }

    return RawBallot{/* bases = */ bases,
                     /* choices = */ choices};
}

Document BallotCodec::decodeRawBallot(const RawBallot & rawBallot) const
{
    // 1. clone the question and reset the selections
    Document question;
    question.CopyFrom(this->question, question.GetAllocator());
    vector<Value *> sortedAnswers =
        sortedAnswersVector(question["answers"], "id");

    // 1.1. perform some format checks in question
    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        throw runtime_error("invalid question format");
    }

    // 1.2. Initialize selection
    for (Value & answer: question["answers"].GetArray())
    {
        answer.AddMember(
            "selected", Value((int32_t) -1), question.GetAllocator());
    }

    // 2. sort & segment answers
    Value * invalidVoteAnswer = nullptr;
    vector<Value *> validAnswers;
    vector<Value *> writeInAnswers;
    for (Value * answer: sortedAnswers)
    {
        if (answerHasUrl(*answer, "invalidVoteFlag"))
        {
            invalidVoteAnswer = answer;
            if (!invalidVoteAnswer->IsObject())
            {
                throw runtime_error("invalid answer is not an object");
            }

            if (rawBallot.choices[0] > 0)
            {
                (*invalidVoteAnswer)["selected"] = 0;
            } else
            {
                (*invalidVoteAnswer)["selected"] = -1;
            }
        } else
        {
            validAnswers.push_back(answer);
        }
        if (answerHasUrl(*answer, "isWriteIn"))
        {
            writeInAnswers.push_back(answer);
        }
    }
    // 4. Do some verifications on the number of choices: Checking that the
    //    raw_ballot has as many choices as required
    size_t minNumChoices = question["answers"].GetArray().Size();
    if (rawBallot.choices.size() < minNumChoices)
    {
        throw runtime_error("Invalid Ballot: Not enough choices to decode");
    }

    // 5. Populate the valid answers. We asume they are in the same order as in
    //    raw_ballot["choices"]
    size_t index = 0;
    for (Value * answer: validAnswers)
    {
        // we add 1 to the index because raw_ballot.choice[0] is just the
        // invalidVoteFlag
        size_t choiceIndex = index + 1;
        int32_t choiceValue =
            static_cast<int32_t>(rawBallot.choices[choiceIndex]);
        (*answer)["selected"] = choiceValue - 1;

        index++;
    }
    // 6. Decode the write-in texts into UTF-8 and split by the \0 character,
    //    finally the text for the write-ins.
    if (question.HasMember("extra_options") &&
        question["extra_options"].IsObject() &&
        question["extra_options"].HasMember("allow_writeins") &&
        question["extra_options"]["allow_writeins"].IsBool() &&
        question["extra_options"]["allow_writeins"].GetBool() == true)
    {
        // if no write ins, return
        if (writeInAnswers.empty())
        {
            return question;
        }
        // 6.1. Slice the choices to get only the bytes related to the write ins
        size_t writeInsStartIndex =
            (invalidVoteAnswer == nullptr)
                ? question["answers"].GetArray().Size() + 1
                : question["answers"].GetArray().Size();

        vector<uint8_t> writeInRawBytes(
            rawBallot.choices.begin() + writeInsStartIndex,
            rawBallot.choices.end());

        // 6.2. Split the write-in bytes arrays in multiple sub-arrays using
        // byte \0 as a separator.
        vector<vector<uint8_t>> writeInsRawBytesArray;
        size_t index2 = 0;

        // initialize
        writeInsRawBytesArray.push_back(vector<uint8_t>());

        for (const uint8_t & byteElement: writeInRawBytes)
        {
            if (byteElement == 0)
            {
                // Start the next write-in byte array, but only if this is
                // not the last one
                if (index2 != writeInRawBytes.size() - 1)
                {
                    writeInsRawBytesArray.push_back(vector<uint8_t>());
                }
            } else
            {
                size_t lastIndex = writeInsRawBytesArray.size() - 1;
                writeInsRawBytesArray[lastIndex].push_back(byteElement);
            }
            index2++;
        }

        if (writeInsRawBytesArray.size() != writeInAnswers.size())
        {
            stringstream error;
            error << "Invalid Ballot: invalid number of write-in bytes,"
                  << " len(writeInsRawBytesArray) = "
                  << writeInsRawBytesArray.size()
                  << ", len(writeInAnswers) = " << writeInAnswers.size()
                  << ", question = " << stringify(question);
            throw runtime_error(error.str());
        }

        // 6.3. Decode each write-in byte array
        vector<string> writeInsDecoded;
        for (const vector<uint8_t> & writeInEncodedUtf8: writeInsRawBytesArray)
        {
            const string writeInDecoded(
                writeInEncodedUtf8.begin(), writeInEncodedUtf8.end());
            writeInsDecoded.push_back(writeInDecoded);
        }

        // 6.4. Assign the write-in name for each write in
        size_t index3 = 0;
        for (Value * writeInAnswer: writeInAnswers)
        {
            if (!writeInAnswer->HasMember("text"))
            {
                writeInAnswer->AddMember(
                    "text",
                    Value().SetString(
                        writeInsDecoded[index3].c_str(),
                        question.GetAllocator()),
                    question.GetAllocator());
            } else
            {
                (*writeInAnswer)["text"].SetString(
                    writeInsDecoded[index3].c_str(), question.GetAllocator());
            }
            index3++;
        }
    } else
    {
        // if there are no write-ins, we will check that there are no more
        // choices set after the choice for the last answer, as they would not
        // mean anything and thus it would be an invalid ballot, but one of a
        // different type that just marking the ballot invalid or marking
        // more/less options than required. It would be gibberish without any
        // meaning, so we raise an exception on that use-case.
        if (validAnswers.size() + 1 != rawBallot.choices.size())
        {
            stringstream error;
            error << "Invalid Ballot: invalid number of choices,"
                  << " len(raw_ballot[\"choices\"]) = "
                  << rawBallot.choices.size()
                  << ", len(valid_answers) + 1 = " << (validAnswers.size() + 1);
            throw runtime_error(error.str());
        }
    }

    return question;
}

}  // namespace BallotVerifier
