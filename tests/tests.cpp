// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <agora-airgap/ElGamal.h>
#include <agora-airgap/common.h>
#include <agora-airgap/encrypt.h>
#include <agora-airgap/sha256.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/elgamal.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <gmock/gmock.h>
#include <gmpxx.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace AgoraAirgap::sha256;
using namespace CryptoPP;
using ::rapidjson::Document;
using ::rapidjson::Value;
using ::std::endl;
using ::std::function;
using ::std::string;
using ::std::stringstream;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::PrintToString;
using ::testing::ThrowsMessage;

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(Sha256Test, StringOne)
{
    BYTE text1[] = {"abc"};
    BYTE hash1[SHA256_BLOCK_SIZE] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40,
        0xde, 0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17,
        0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, text1, sizeof(text1) - 1);
    sha256_final(&ctx, buf);
    EXPECT_EQ(0, memcmp(hash1, buf, SHA256_BLOCK_SIZE));
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(Sha256Test, StringTwo)
{
    BYTE text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
    BYTE hash2[SHA256_BLOCK_SIZE] = {
        0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26,
        0x93, 0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff,
        0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, text2, sizeof(text2) - 1);
    sha256_final(&ctx, buf);
    EXPECT_EQ(0, memcmp(hash2, buf, SHA256_BLOCK_SIZE));
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(Sha256Test, StringLoopTest)
{
    BYTE text3[] = {"aaaaaaaaaa"};
    BYTE hash3[SHA256_BLOCK_SIZE] = {
        0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92, 0x81, 0xa1, 0xc7,
        0xe2, 0x84, 0xd7, 0x3e, 0x67, 0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97,
        0x20, 0x0e, 0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0};
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    for (int idx = 0; idx < 100000; ++idx)
    {
        sha256_update(&ctx, text3, sizeof(text3) - 1);
    }
    sha256_final(&ctx, buf);
    EXPECT_EQ(0, memcmp(hash3, buf, SHA256_BLOCK_SIZE));
}

// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(AgoraUnitTest, SmallMessageEncryption)
{
    mpz_t p, q, g, y, x, m, rand;

    mpz_init_set_str(p, "1019", 10);
    mpz_init_set_str(q, "509", 10);  // q = (p-1)/2 = 509
    mpz_init_set_str(g, "3", 10);
    mpz_init_set_str(y, "770", 10);  // y = (g^x) mod p
    mpz_init_set_str(x, "66", 10);
    mpz_init_set_str(m, "133", 10);
    mpz_init_set_str(rand, "4", 10);

    mpz_class p_c(p), q_c(q), g_c(g), x_c(x), y_c(y), rand_c(rand), m_c(m);

    AgoraAirgap::ElGamal::PublicKey pk(p_c, q_c, g_c, y_c);
    AgoraAirgap::ElGamal::Plaintext plaintext(m_c, pk, true);
    AgoraAirgap::ElGamal::Ciphertext ciphertext =
        AgoraAirgap::ElGamal::encrypt(pk, plaintext, rand_c);
    // alpha = (g^random) mod p  = (3^4) mod 1019 = 81
    // U = (y^random) mod p = (770^4) mod 1019 = 508
    // M = m+1 = 134
    // beta = ( U M  ) mod p = ( 508 * 134) mod 1019 = 818

    EXPECT_EQ(0, ciphertext.alpha.get_str().compare("81"));
    EXPECT_EQ(0, ciphertext.beta.get_str().compare("818"));
}

/**
 * Executes download_audit with invalid data (/config-bad will yield nothing)
 */
// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST(Example1, MockBadDownloadAudit)
{
    string examplePath = "example_1";
    string ballotPath = examplePath + "/auditable_ballot.json";
    auto getConfig = [&examplePath](stringstream & out, const string &) {
        return AgoraAirgap::read_file(out, examplePath + "/config-bad");
    };
    stringstream out;
    EXPECT_THAT(
        [&]() { AgoraAirgap::download_audit(out, ballotPath, getConfig); },
        ThrowsMessage<std::runtime_error>(HasSubstr("!!! Error [read-file]")))
        << endl
        << "Error found in output: " << out.str() << endl;
}

/**
 * Groups examples that should not fail
 */
class ExampleDirsTest : public ::testing::Test
{
    protected:
    void SetUp() override
    {
        this->exampleDirs = std::vector<std::string>(
            {"fixtures/example_1",
             "fixtures/example_1__invalid_ballot_json",
             "fixtures/example_1__ballot_hash_error",
             "fixtures/example_1__invalid_choice_randomness",
             "fixtures/example_1__invalid_choice_plaintext",
             "fixtures/example_1__invalid_choice_alpha",
             "fixtures/example_1__invalid_choice_beta"});
    }
    std::vector<string> exampleDirs;
};

/**
 * Helper function to run expectations.json
 */
void runExpectations2(
    const function<string()> & lambda1,
    const function<string()> & lambda2,
    const Document & document,
    const string & testName)
{
    SCOPED_TRACE("run_expectations2: " + testName);
    const Value & config = document[testName.c_str()];
    const string & runType = config["type"].GetString();
    stringstream result1;
    if (runType == string("Equals"))
    {
        const string & runMessage = config["message"].GetString();
        EXPECT_EQ(lambda1(), lambda2()) << runMessage << endl;
    } else if (runType == string("NotEquals"))
    {
        const string & run_message = config["message"].GetString();
        EXPECT_NE(lambda1(), lambda2()) << run_message << endl;
    } else
    {
        FAIL() << "invalid run type: " << runType << endl;
    }
}

/**
 * Helper function to run expectations.json
 */
void runExpectations(
    const function<void(stringstream &)> & lambda,
    const Document & document,
    const string & testName,
    bool * hasAssertion = nullptr)
{
    SCOPED_TRACE("run_expectations: " + testName);
    const Value & config = document[testName.c_str()];
    const string & runType = config["type"].GetString();
    stringstream out;
    if (runType == string("NoThrow"))
    {
        const string & runMessage = config["message"].GetString();

        EXPECT_NO_THROW({ lambda(out); }) << runMessage << endl
                                          << "out = " << out.str() << endl;

    } else if (runType == string("ThrowsMessage_HasSubStr"))
    {
        const string & matchString = config["data"].GetString();
        const string & runMessage = config["message"].GetString();
        EXPECT_THAT(
            [&]() { lambda(out); },
            ThrowsMessage<std::runtime_error>(HasSubstr(matchString)))
            << runMessage << endl;
    } else if (runType == string("ASSERT_ThrowsMessage_HasSubStr"))
    {
        const string & matchString = config["data"].GetString();
        const string & runMessage = config["message"].GetString();
        if (hasAssertion != nullptr)
        {
            *hasAssertion = true;
        }
        ASSERT_THAT(
            [&]() { lambda(out); },
            ThrowsMessage<std::runtime_error>(HasSubstr(matchString)))
            << runMessage << endl;
        if (hasAssertion != nullptr)
        {
            *hasAssertion = true;
        }
    } else
    {
        FAIL() << "invalid run type: " << runType << endl;
    }
    string result = out.str();

    for (const Value & expectation: config["expectations"].GetArray())
    {
        const string & expectationType = expectation["type"].GetString();
        if (expectationType == string("HasSubstr"))
        {
            const string & data = expectation["data"].GetString();
            const string & message = expectation["message"].GetString();
            EXPECT_THAT(result, HasSubstr(data)) << message << endl;
        } else if (expectationType == string("Not_HasSubstr"))
        {
            const string & data = expectation["data"].GetString();
            const string & message = expectation["message"].GetString();
            EXPECT_THAT(result, Not(HasSubstr(data))) << message << endl;
        } else if (expectationType == string("NoThrow"))
        {
            const string & data = expectation["data"].GetString();
            const string & message = expectation["message"].GetString();
            EXPECT_THAT(result, Not(HasSubstr(data))) << message << endl;
        } else if (expectationType == string("Not_HasSubstr"))
        {
            const string & data = expectation["data"].GetString();
            const string & message = expectation["message"].GetString();
            EXPECT_THAT(result, Not(HasSubstr(data))) << message << endl;
        } else
        {
            FAIL() << "invalid expectation type: " << expectationType << endl;
        }
    }
}

/**
 * Handy function to get the expectations doc from a fixture
 */
void getExpectationsDoc(const string & examplePath, Document & expectationsDoc)
{
    const string expectationsPath = examplePath + "/expectations.json";
    stringstream out;
    const string expectationsData =
        AgoraAirgap::read_file(out, expectationsPath.c_str());

    if (expectationsDoc.Parse(expectationsData.c_str()).HasParseError())
    {
        FAIL() << "getExpectationsDoc(\"" << examplePath
               << "\"): parse error:\n"
               << "\toffset: " << (unsigned) expectationsDoc.GetErrorOffset()
               << "\n\terror: "
               << GetParseError_En(expectationsDoc.GetParseError()) << endl;
    }
}

/**
 * Executes download_audit with correct data
 */
// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST_F(ExampleDirsTest, MockDownloadAudit)
{
    for (string & examplePath: exampleDirs)
    {
        SCOPED_TRACE(examplePath);
        std::cout << "examplePath: " << examplePath << endl;
        Document expectationsDoc;
        getExpectationsDoc(examplePath, expectationsDoc);
        string ballotPath = examplePath + "/auditable_ballot.json";
        auto getConfig = [&examplePath](stringstream & out, const string &) {
            return AgoraAirgap::read_file(out, examplePath + "/config");
        };

        runExpectations(
            [&](stringstream & out) {
                AgoraAirgap::download_audit(out, ballotPath, getConfig);
            },
            expectationsDoc,
            "MockDownloadAudit");
    }
}

/**
 * Executes download only and compares downloaded file with original
 */
// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST_F(ExampleDirsTest, MockDownload)
{
    SCOPED_TRACE("");
    for (string & examplePath: exampleDirs)
    {
        Document expectationsDoc;
        getExpectationsDoc(examplePath, expectationsDoc);
        string ballotPath = examplePath + "/auditable_ballot.json";
        auto getConfig = [&examplePath](stringstream & out, const string &) {
            return AgoraAirgap::read_file(out, examplePath + "/config");
        };
        string electionPath = std::tmpnam(nullptr);

        bool hasAssertion = false;
        runExpectations(
            [&](stringstream & out) {
                AgoraAirgap::download(out, ballotPath, electionPath, getConfig);
            },
            expectationsDoc,
            "MockDownload::Run",
            &hasAssertion);

        if (hasAssertion)
        {
            return;
        }

        stringstream out2;
        runExpectations2(
            [&]() { return AgoraAirgap::read_file(out2, electionPath); },
            [&]() {
                return AgoraAirgap::read_file(out2, examplePath + "/config");
            },
            expectationsDoc,
            "MockDownload::Compare");
        runExpectations(
            [&](stringstream & out) { out << out2.str(); },
            expectationsDoc,
            "MockDownload::CompareOutput");
    }
}

/**
 * Executes audit only
 */
// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST_F(ExampleDirsTest, MockAudit)
{
    for (string & examplePath: exampleDirs)
    {
        SCOPED_TRACE(examplePath);
        Document expectationsDoc;
        getExpectationsDoc(examplePath, expectationsDoc);
        string ballotPath = examplePath + "/auditable_ballot.json";
        stringstream out;
        string electionPath = examplePath + "/config";

        runExpectations(
            [&](stringstream & out) {
                AgoraAirgap::audit(out, ballotPath, electionPath);
            },
            expectationsDoc,
            "MockAudit");
    }
}

/**
 * Executes audit only
 */
// Supress warnings related to using the google test macro
// NOLINTNEXTLINE(misc-unused-parameters, readability-named-parameter)
TEST_F(ExampleDirsTest, EncryptAndAudit)
{
    this->exampleDirs = std::vector<std::string>({"fixtures/example_1"});

    for (string & examplePath: exampleDirs)
    {
        SCOPED_TRACE(examplePath);
        Document expectationsDoc;
        getExpectationsDoc(examplePath, expectationsDoc);
        string plainTextVotesPath = examplePath + "/plaintext_vote.json";
        string electionPath = examplePath + "/config";
        string encryptedBallotPath = std::tmpnam(nullptr);

        runExpectations(
            [&](stringstream & out) {
                AgoraAirgap::encrypt_ballot(
                    out, plainTextVotesPath, electionPath, encryptedBallotPath);
            },
            expectationsDoc,
            "EncryptAndAudit::Encrypt");

        runExpectations(
            [&](stringstream & out) {
                AgoraAirgap::audit(out, encryptedBallotPath, electionPath);
            },
            expectationsDoc,
            "EncryptAndAudit::Audit");
    }
}
