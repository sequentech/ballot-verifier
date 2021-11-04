// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/NVotesCodec.h>
#include <agora-airgap/common.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace AgoraAirgap;
using AgoraAirgap::NVotesCodec;
using AgoraAirgap::RawBallot;
using rapidjson::Document;
using std::endl;
using std::runtime_error;
using std::string;
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

    // The question contains the minimum data required for the encoder to work
    const TestData fixture[] = {
        TestData{/* question = */ R"({
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
                 /* bases = */ {2, 2, 2, 2, 2, 2, 2, 2}},
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0}
                ]
            })",
                 /* bases = */ {2, 2}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "max": 1,
                "answers": [
                    {"id": 0}
                ]
            })",
                 /* bases = */ {2, 2}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "max": 2,
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2}
                ]
            })",
                 /* bases = */ {2, 3, 3, 3}}};

    for (const TestData & testData: fixture)
    {
        Document questionDoc;
        EXPECT_FALSE(
            questionDoc.Parse(testData.question.c_str()).HasParseError())
            << "Parse error in the fixture" << endl;

        NVotesCodec codec(questionDoc);
        EXPECT_EQ(codec.getBases(), testData.bases) << "Invalid bases" << endl;
    }
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(NVotesCodec, EncodeRawBallot)
{
    class TestData
    {
        public:
        const string question;
        const vector<uint32_t> bases;
        const vector<uint32_t> choices;
    };

    // The question contains the minimum data required for the encoder to work
    const TestData fixture[] = {
        TestData{/* question = */ R"({
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
                 /* bases = */ {2, 2, 2, 2, 2, 2, 2, 2},
                 /* choices = */ {0, 0, 1, 0, 0, 0, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": 0},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5, "selected": 1},
                    {"id": 6}
                ]
            })",
                 /* bases = */ {2, 2, 2, 2, 2, 2, 2, 2},
                 /* choices = */ {0, 1, 1, 0, 0, 0, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "max": 3,
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": 2},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5, "selected": 1},
                    {"id": 6}
                ]
            })",
                 /* bases = */ {2, 4, 4, 4, 4, 4, 4, 4},
                 /* choices = */ {0, 1, 3, 0, 0, 0, 2, 0}},
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1},
                    {
                        "id": 2, 
                        "selected": 1,
                        "urls": [
                            {"title": "invalidVoteFlag", "url": "true"}
                        ]
                    }
                ]
            })",
                 /* bases = */ {2, 2, 2},
                 /* choices = */ {1, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "max": 2,
                "extra_options": {"allow_writeins": true},
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1},
                    {"id": 2},
                    {
                        "id": 3, 
                        "selected": 0,
                        "urls": [
                            {"title": "invalidVoteFlag", "url": "true"}
                        ]
                    },
                    {
                        "id": 4, 
                        "text": "D",
                        "selected": 1,
                        "urls": [
                            {"title": "isWriteIn", "url": "true"}
                        ]
                    },
                    {
                        "id": 5, 
                        "text": "",
                        "urls": [
                            {"title": "isWriteIn", "url": "true"}
                        ]
                    }
                ]
            })",
                 /* bases = */ {2, 3, 3, 3, 3, 3, 256, 256, 256},
                 /* choices = */ {1, 1, 0, 0, 2, 0, 68, 0, 0}},
    };

    /*
          dict(
            question=dict(
              tally_type="borda",
              max=2,
              extra_options=dict(allow_writeins=True),
              answers=[
                dict(id=0, selected=0),
                dict(id=1),
                dict(id=2),
                dict(
                  id=3,
                  selected=0,
                  urls=[dict(title='invalidVoteFlag', url='true')]
                ),
                dict(
                  id=4,
                  text='D',
                  selected=1,
                  urls=[dict(title='isWriteIn', url='true')]
                ),
                dict(
                  id=5,
                  text='',
                  urls=[dict(title='isWriteIn', url='true')]
                )
              ]
            ),
            bases=     [2, 3, 3, 3, 3, 3, 256, 256, 256],
            choices=   [1, 1, 0, 0, 2, 0, 68,  0,   0]
          ),
          dict(
            question=dict(
              tally_type="plurality-at-large",
              extra_options=dict(allow_writeins=True),
              max=3,
              answers=[
                dict(id=0, selected=1),
                dict(id=1),
                dict(id=2),
                dict(
                  id=3,
                  urls=[dict(title='invalidVoteFlag', url='true')]
                ),
                dict(
                  id=4,
                  text='E',
                  selected=1,
                  urls=[dict(title='isWriteIn', url='true')]
                ),
                dict(
                  id=5,
                  text='',
                  urls=[dict(title='isWriteIn', url='true')]
                ),
                dict(
                  id=6,
                  selected=1,
                  text='Ä bc',
                  urls=[dict(title='isWriteIn', url='true')]
                )
              ]
            ),
            bases=    [2, 2, 2, 2, 2, 2, 2, 256, 256, 256, 256, 256, 256, 256,
       256, 256], choices=  [0, 1, 0, 0, 1, 0, 1, 69,  0,   0,   195, 132, 32,
       98,  99,  0]
          ),
    */

    for (const TestData & testData: fixture)
    {
        Document questionDoc;
        EXPECT_FALSE(
            questionDoc.Parse(testData.question.c_str()).HasParseError())
            << "Parse error in the fixture" << endl;
        // TODO: self.assertTrue(codec.sanity_check())

        NVotesCodec codec(questionDoc);
        RawBallot rawBallot = codec.encodeRawBallot();
        EXPECT_EQ(rawBallot.bases, testData.bases) << "Invalid bases" << endl;

        EXPECT_EQ(rawBallot.choices, testData.choices)
            << "Invalid choices" << endl;
    }
}