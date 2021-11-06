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
using rapidjson::StringBuffer;
using rapidjson::Writer;
using std::endl;
using std::runtime_error;
using std::string;
using std::vector;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

string stringify(const Document & doc)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

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
        TestData{
            /* question = */ R"({
                "tally_type": "plurality-at-large",
                "max": 3,
                "extra_options": {"allow_writeins": true},
                "answers": [
                    {"id": 0, "selected": 1},
                    {"id": 1},
                    {"id": 2},
                    {
                        "id": 3,
                        "urls": [
                            {"title": "invalidVoteFlag", "url": "true"}
                        ]
                    },
                    {
                        "id": 4, 
                        "text": "E",
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
                    },
                    {
                        "id": 6, 
                        "text": "Ä bc",
                        "selected": 1,
                        "urls": [
                            {"title": "isWriteIn", "url": "true"}
                        ]
                    }
                ]
            })",
            /* bases = */
            {2, 2, 2, 2, 2, 2, 2, 256, 256, 256, 256, 256, 256, 256, 256, 256},
            /* choices = */
            {0, 1, 0, 0, 1, 0, 1, 69, 0, 0, 195, 132, 32, 98, 99, 0}}};

    for (const TestData & testData: fixture)
    {
        Document questionDoc;
        EXPECT_FALSE(
            questionDoc.Parse(testData.question.c_str()).HasParseError())
            << "Parse error in the fixture" << endl;

        NVotesCodec codec(questionDoc);
        RawBallot rawBallot = codec.encodeRawBallot();
        EXPECT_EQ(rawBallot.bases, testData.bases) << "Invalid bases" << endl;

        EXPECT_EQ(rawBallot.choices, testData.choices)
            << "Invalid choices" << endl;
    }
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(NVotesCodec, DecodeRawBallot)
{
    class TestData
    {
        public:
        const string question;
        const string decodedBallot;
        const vector<uint32_t> bases;
        const vector<uint32_t> choices;
    };

    // The question contains the minimum data required for the encoder to work
    const TestData fixture[] = {
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5},
                    {"id": 6}
                ]
            })",
                 /* decodedBallot = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0, "selected":-1},
                    {"id": 1, "selected": 0},
                    {"id": 2, "selected":-1},
                    {"id": 3, "selected":-1},
                    {"id": 4, "selected":-1},
                    {"id": 5, "selected": 0},
                    {"id": 6, "selected":-1}
                ]
            })",
                 /* bases   = */ {2, 2, 2, 2, 2, 2, 2, 2},
                 /* choices = */ {0, 0, 1, 0, 0, 0, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5},
                    {"id": 6}
                ]
            })",
                 /* decodedBallot = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": 0},
                    {"id": 2, "selected":-1},
                    {"id": 3, "selected":-1},
                    {"id": 4, "selected":-1},
                    {"id": 5, "selected": 0},
                    {"id": 6, "selected":-1}
                ]
            })",
                 /* bases   = */ {2, 2, 2, 2, 2, 2, 2, 2},
                 /* choices = */ {0, 1, 1, 0, 0, 0, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2},
                    {"id": 3},
                    {"id": 4},
                    {"id": 5},
                    {"id": 6}
                ]
            })",
                 /* decodedBallot = */ R"({
                "tally_type": "borda",
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": 2},
                    {"id": 2, "selected":-1},
                    {"id": 3, "selected":-1},
                    {"id": 4, "selected":-1},
                    {"id": 5, "selected": 1},
                    {"id": 6, "selected":-1}
                ]
            })",
                 /* bases   = */ {2, 4, 4, 4, 4, 4, 4, 4},
                 /* choices = */ {0, 1, 3, 0, 0, 0, 2, 0}},
        TestData{/* question = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {
                      "id": 2,
                      "urls": [{"title": "invalidVoteFlag", "url": "true"}]
                    }
                ]
            })",
                 /* decodedBallot = */ R"({
                "tally_type": "plurality-at-large",
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": -1},
                    {
                      "id": 2,
                      "urls": [{"title": "invalidVoteFlag", "url": "true"}],
                      "selected": 0
                    }
                ]
            })",
                 /* bases   = */ {2, 2, 2},
                 /* choices = */ {1, 1, 0}},
        TestData{/* question = */ R"({
                "tally_type": "borda",
                "max": 2,
                "extra_options": {"allow_writeins": true},
                "answers": [
                    {"id": 0},
                    {"id": 1},
                    {"id": 2},
                    {
                      "id": 3,
                      "urls": [{"title": "invalidVoteFlag", "url": "true"}]
                    },
                    {
                      "id": 4,
                      "urls": [{"title": "isWriteIn", "url": "true"}]
                    },
                    {
                      "id": 5,
                      "urls": [{"title": "isWriteIn", "url": "true"}]
                    }
                ]
            })",
                 /* decodedBallot = */ R"({
                "tally_type": "borda",
                "max": 2,
                "extra_options": {"allow_writeins": true},
                "answers": [
                    {"id": 0, "selected": 0},
                    {"id": 1, "selected": -1},
                    {"id": 2, "selected": -1},
                    {
                      "id": 3,
                      "urls": [{"title": "invalidVoteFlag", "url": "true"}],
                      "selected": 0
                    },
                    {
                      "id": 4,
                      "urls": [{"title": "isWriteIn", "url": "true"}],
                      "selected": 1,
                      "text": "D"
                    },
                    {
                      "id": 5,
                      "urls": [{"title": "isWriteIn", "url": "true"}],
                      "selected": -1,
                      "text": ""
                    }
                ]
            })",
                 /* bases   = */ {2, 3, 3, 3, 3, 3, 256, 256, 256},
                 /* choices = */ {1, 1, 0, 0, 2, 0, 68, 0, 0}}};

    for (const TestData & testData: fixture)
    {
        Document questionDoc;
        EXPECT_FALSE(
            questionDoc.Parse(testData.question.c_str()).HasParseError())
            << "Parse error in the fixture question" << endl;
        Document expectedDecodedBallot;
        EXPECT_FALSE(expectedDecodedBallot.Parse(testData.decodedBallot.c_str())
                         .HasParseError())
            << "Parse error in the fixture decodedBallot" << endl;

        NVotesCodec codec(questionDoc);

        // check raw ballot getter
        Document decodedBallot =
            codec.decodeRawBallot(RawBallot{/* bases = */ testData.bases,
                                            /* choices = */ testData.choices});

        EXPECT_EQ(stringify(decodedBallot), stringify(expectedDecodedBallot))
            << "Invalid decodedBallot" << endl;
    }
}

/*
  def test_decode_raw_ballot(self):
    # The question contains the minimum data required for the encoder to work
    data_list = [
      dict(
        question=dict(
          tally_type="borda",
          max=2,
          extra_options=dict(allow_writeins=True),
          answers=[
            dict(id=0),
            dict(id=1),
            dict(id=2),
            dict(
              id=3,
              urls=[dict(title='invalidVoteFlag', url='true')]
            ),
            dict(
              id=4,
              urls=[dict(title='isWriteIn', url='true')]
            ),
            dict(
              id=5,
              urls=[dict(title='isWriteIn', url='true')]
            )
          ]
        ),
        decoded_ballot=dict(
          tally_type="borda",
          max=2,
          extra_options=dict(allow_writeins=True),
          answers=[
            dict(id=0, selected=0 ),
            dict(id=1, selected=-1),
            dict(id=2, selected=-1),
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
              selected=-1,
              urls=[dict(title='isWriteIn', url='true')]
            )
          ]
        ),
        bases=  [2, 3, 3, 3, 3, 3, 256, 256, 256],
        choices=[1, 1, 0, 0, 2, 0, 68,  0,   0]
      ),
      dict(
        question=dict(
          tally_type="plurality-at-large",
          extra_options=dict(allow_writeins=True),
          max=3,
          answers=[
            dict(id=0),
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
              text='Ä bc',
              urls=[dict(title='isWriteIn', url='true')]
            )
          ]
        ),
        decoded_ballot=dict(
          tally_type="plurality-at-large",
          extra_options=dict(allow_writeins=True),
          max=3,
          answers=[
            dict(id=0, selected=0 ),
            dict(id=1, selected=-1),
            dict(id=2, selected=-1),
            dict(
              id=3,
              urls=[dict(title='invalidVoteFlag', url='true')]
            ),
            dict(
              id=4,
              text='E',
              selected=0,
              urls=[dict(title='isWriteIn', url='true')]
            ),
            dict(
              id=5,
              text='',
              selected=-1,
              urls=[dict(title='isWriteIn', url='true')]
            ),
            dict(
              id=6,
              selected=0,
              text='Ä bc',
              urls=[dict(title='isWriteIn', url='true')]
            )
          ]
        ),
        bases=  [2, 2, 2, 2, 2, 2, 2, 256, 256, 256, 256, 256, 256, 256, 256,
  256], choices=[0, 1, 0, 0, 1, 0, 1, 69,  0,   0,   195, 132, 32,  98,  99,  0]
      ),
    ]
    for data in data_list:
      codec = NVotesCodec(data["question"])

      # check raw ballot getter
      decoded_ballot = codec.decode_raw_ballot(dict(
        bases=data['bases'],
        choices=data['choices']
      ))
      self.assertEqual(
        decoded_ballot,
        data['decoded_ballot']
      )
*/