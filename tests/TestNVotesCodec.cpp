// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/NVotesCodec.h>
#include <agora-airgap/common.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>
#include <string>

using namespace AgoraAirgap;
using AgoraAirgap::NVotesCodec;
using rapidjson::Document;
using std::endl;
using std::string;
using std::runtime_error;
using std::vector;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(NVotesCodec, Bases)
{
    class TestData
    {
        public:
        const string question;
        const vector<uint32_t> bases;
    };

    const TestData fixture[] = {
        TestData {
            /* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0},
                    {"id": 1, "selected": 0},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5, "selected": 1},
                    {"id": 6}
                ]
            })",
            /* bases = */ {2, 2, 2, 2, 2, 2, 2, 2}
        },
        TestData {
            /* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0}
                ]
            })",
            /* bases = */ {2, 2}
        },
        TestData {
            /* question = */ R"({
                "tally_type": "borda",
                "max": 1,
                "answers": [
                    {"id": 0}
                ]
            })",
            /* bases = */ {2, 2}
        },
        TestData {
            /* question = */ R"({
                "tally_type": "borda",
                "max": 2,
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2}
                ]
            })",
            /* bases = */ {2, 3, 3, 3}
        }
    };

    for (const TestData & testData: fixture)
    {
        Document questionDoc;
        EXPECT_FALSE(questionDoc.Parse(testData.question.c_str()).HasParseError())
            << "Parse error in the fixture" << endl;
        
        NVotesCodec codec(questionDoc);
        EXPECT_TRUE(codec.getBases() == testData.bases)
            << "Invalid bases" << endl;
    }
}
