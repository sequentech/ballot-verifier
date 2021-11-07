// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Based on Eduardo Robles's agora-api:
// https://github.com/agoravoting/agora-api

#include <gmpxx.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#define CURL_STATICLIB
#endif
#include <agora-airgap/ElGamal.h>
#include <agora-airgap/NVotesCodec.h>
#include <agora-airgap/Random.h>
#include <agora-airgap/encrypt.h>
#include <curl/curl.h>

using namespace rapidjson;

namespace AgoraAirgap {

string get_date()
{
    time_t now = time(0);
    tm * gmtm = gmtime(&now);
    stringstream ss;  //"2014-11-24"
    ss << gmtm->tm_mday << "/" << gmtm->tm_mon << "/" << (1900 + gmtm->tm_year);
    return ss.str();
}

string stringify(const Value & d)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}

string read_file(stringstream & out, const string & path)
{
    ifstream t(path.c_str());

    if (!t.good())
    {
        out << "!!! Error [read-file]: error opening file: " << path << endl;
        throw runtime_error(out.str());
    }
    string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign(
        (std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

bool save_file(stringstream & out, const string & path, const string & text)
{
    out << "> writing to file " + path << endl;
    ofstream f(path.c_str());
    bool ret = false;
    if (f.good())
    {
        f << text;
        f.close();
        ret = true;
    } else
    {
        f.close();
    }
    return ret;
}

Document encryptAnswer(
    stringstream & out,
    const Value & pk_json,
    const mpz_class & plain_vote,
    rapidjson::Value::AllocatorType & allocator)
{
    ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json);
    ElGamal::Plaintext plaintext(plain_vote, pk, true);
    mpz_class randomness = Random::getRandomIntegerRange(pk.q);
    out << "> plaintext = " << plain_vote.get_str(10)
        << "\n> randomness = " << randomness.get_str(10) << endl;
    ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, randomness);
    ElGamal::Fiatshamir_dlog_challenge_generator
        fiatshamir_dlog_challenge_generator;
    ElGamal::DLogProof proof = plaintext.proveKnowledge(
        ctext.alpha, randomness, fiatshamir_dlog_challenge_generator);

    bool verified =
        ctext.verifyPlaintextProof(proof, fiatshamir_dlog_challenge_generator);

    Document encryptedAnswer;
    encryptedAnswer.SetObject();
    encryptedAnswer.AddMember(
        "alpha",
        Value().SetString(ctext.alpha.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "beta",
        Value().SetString(ctext.beta.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "commitment",
        Value().SetString(proof.commitment.a.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "response",
        Value().SetString(proof.response.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "challenge",
        Value().SetString(proof.challenge.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "randomness",
        Value().SetString(randomness.get_str(10).c_str(), allocator),
        allocator);
    encryptedAnswer.AddMember(
        "plaintext",
        Value().SetString(plain_vote.get_str(10).c_str(), allocator),
        allocator);

    out << "> Node: proof verified = " << (verified ? "true" : "false") << endl;

    return encryptedAnswer;
}

string generateBallotHash(const rapidjson::Document & originalBallot)
{
    rapidjson::Document ballot;
    ballot.CopyFrom(originalBallot, ballot.GetAllocator());

    ballot.RemoveMember("election_url");
    ballot.RemoveMember("ballot_hash");
    for (SizeType i = 0; i < ballot["choices"].Size(); ++i)
    {
        ballot["choices"][i].RemoveMember("plaintext");
        ballot["choices"][i].RemoveMember("randomness");
    }

    stringstream ballotss;
    string partial;

    Document choices_doc;
    choices_doc.SetObject();
    choices_doc.AddMember(
        "choices", ballot["choices"], choices_doc.GetAllocator());
    partial = stringify(choices_doc);
    partial = partial.substr(1, partial.size() - 2);

    ballotss << "{" << partial << ","
             << "\"issue_date\":\"" << ballot["issue_date"].GetString()
             << "\",";

    Document proofs_doc;
    proofs_doc.SetObject();
    proofs_doc.AddMember("proofs", ballot["proofs"], proofs_doc.GetAllocator());
    partial = stringify(proofs_doc);
    partial = partial.substr(1, partial.size() - 2);

    ballotss << partial << "}";

    return sha256::hex_sha256(ballotss.str());
}

void encrypt_ballot(
    stringstream & out,
    const string & plaintextVotesPath,
    const string & configPath,
    const string & ballotPath)
{
    try
    {
        Document electionConfig, ballot, plaintextVotes, publicKeys;

        out << "> reading plaintext ballot" << endl;
        if (plaintextVotes.Parse(read_file(out, plaintextVotesPath).c_str())
                .HasParseError() ||
            !plaintextVotes.IsArray())
        {
            out << "!!! Error [reading-plaintext-json]: Json format error"
                << endl;
            throw runtime_error(out.str());
        }

        out << "> reading config file" << endl;
        if (electionConfig.Parse(read_file(out, configPath).c_str())
                .HasParseError())
        {
            out << "!!! Error [reading-config-json]: Parse Json format error"
                << endl;
            throw runtime_error(out.str());
        }
        if (!electionConfig.IsObject() ||
            !electionConfig.HasMember("payload") ||
            !electionConfig["payload"].IsObject() ||
            !electionConfig["payload"].HasMember("pks"))
        {
            out << "electionConfig = " << stringify(electionConfig)
                << "!!! Error [reading-config-json2]: Json payload format error"
                << endl;
            throw runtime_error(out.str());
        }

        if (electionConfig["payload"]["pks"].IsString())
        {
            const string publicKeysString =
                electionConfig["payload"]["pks"].GetString();
            if (publicKeys.Parse(publicKeysString.c_str()).HasParseError())
            {
                out << "!!! Error [parsing-pks-json]: Json format error"
                    << endl;
                throw runtime_error(out.str());
            }
        } else if (electionConfig["payload"]["pks"].IsArray())
        {
            publicKeys.CopyFrom(
                electionConfig["payload"]["pks"], publicKeys.GetAllocator());
        } else
        {
            out << "!!! Error [parsing-pks-json2]: Json format error" << endl;
            throw runtime_error(out.str());
        }

        if (plaintextVotes.GetArray().Size() != publicKeys.GetArray().Size())
        {
            out << "!!! Error [reading-pks-size]: Invalid size" << endl;
            throw runtime_error(out.str());
        }

        out << "> generating encrypted ballot" << endl;

        const Value & plaintextVotesArray = plaintextVotes;

        ballot.SetObject();
        ballot.AddMember(
            "election_url",
            "http://0.0.0.0:8000/config",
            ballot.GetAllocator());
        ballot.AddMember(
            "issue_date",
            Value().SetString(get_date().c_str(), ballot.GetAllocator()),
            ballot.GetAllocator());
        ballot.AddMember("choices", Value().SetArray(), ballot.GetAllocator());
        ballot.AddMember("proofs", Value().SetArray(), ballot.GetAllocator());

        size_t index = 0;
        for (const Value & plaintextQuestion: plaintextVotesArray.GetArray())
        {
            // obtain plaintext big-int from the json ballot
            Document plaintextQuestionDoc;
            plaintextQuestionDoc.CopyFrom(
                plaintextQuestion, plaintextQuestionDoc.GetAllocator());
            AgoraAirgap::NVotesCodec codec(plaintextQuestionDoc);
            RawBallot rawBallot = codec.encodeRawBallot();
            mpz_class plaintext = codec.encodeToInt(rawBallot);

            const Value & publicKey = publicKeys[index];

            Document encryptedAnswer =
                encryptAnswer(out, publicKey, plaintext, ballot.GetAllocator());
            Value choice;
            choice.SetObject();
            choice.AddMember(
                "alpha", encryptedAnswer["alpha"], ballot.GetAllocator());
            choice.AddMember(
                "beta", encryptedAnswer["beta"], ballot.GetAllocator());
            choice.AddMember(
                "plaintext",
                encryptedAnswer["plaintext"],
                ballot.GetAllocator());
            choice.AddMember(
                "randomness",
                encryptedAnswer["randomness"],
                ballot.GetAllocator());
            Value proof;
            proof.SetObject();
            proof.AddMember(
                "challenge",
                encryptedAnswer["challenge"],
                ballot.GetAllocator());
            proof.AddMember(
                "commitment",
                encryptedAnswer["commitment"],
                ballot.GetAllocator());
            proof.AddMember(
                "response", encryptedAnswer["response"], ballot.GetAllocator());
            ballot["choices"].PushBack(choice.Move(), ballot.GetAllocator());
            ballot["proofs"].PushBack(proof.Move(), ballot.GetAllocator());
            index++;
        }

        // Only after everything has been set in the ballot, we calculate the
        // ballot hash
        const string ballotHash = generateBallotHash(ballot);
        ballot.AddMember(
            "ballot_hash",
            Value().SetString(ballotHash.c_str(), ballot.GetAllocator()),
            ballot.GetAllocator());

        out << "> saving encrypted ballot to file..." << endl;
        if (!save_file(out, ballotPath, stringify(ballot)))
        {
            out << "!!! Error [encrypt-ballot-save]: couldn't save encrypted "
                << "ballot to file path " << ballotPath << endl;
        }
    } catch (...)
    {
        out << "!!! Error [encrypt-ballot-catch]: " << out.str() << endl;
        throw runtime_error(out.str());
    }
}

static size_t
WriteCallback(void * contents, size_t size, size_t nmemb, void * userp)
{
    ((std::string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

string download_url(stringstream & out, const string & url)
{
    CURL * curl;
    CURLcode res;
    string read_buffer;

    curl = curl_easy_init();
    if (!curl)
    {
        out << "!!! Error [download-url-curl-not-found]: curl doesn't work"
            << std::endl;
        throw runtime_error(out.str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        out << "!!! Error [download-url-curl2]: curl_easy_perform() failed:"
            << curl_easy_strerror(res) << endl;
        throw runtime_error(out.str());
    }

    curl_easy_cleanup(curl);
    return read_buffer;
}

template <typename T>
string to_string(T i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

/// @returns the ballot corresponding for the plaintext and the question
Document parseBallot(const Value & question, const string & plaintextString)
{
    mpz_class plaintext(plaintextString.c_str());

    Document questionDoc;
    questionDoc.CopyFrom(question, questionDoc.GetAllocator());

    AgoraAirgap::NVotesCodec codec(questionDoc);

    AgoraAirgap::RawBallot rawBallot = codec.decodeFromInt(plaintext);
    return codec.decodeRawBallot(rawBallot);
}

bool isInvalidBallot(const Document & ballot)
{
    for (const Value & answer: ballot["answers"].GetArray())
    {
        if (AgoraAirgap::answerHasUrl(answer, "invalidVoteFlag"))
        {
            return true;
        }
    }
    return false;
}

bool isSortedQuestionType(const Document & ballot)
{
    const string tallyType = ballot["tally_type"].GetString();
    return tallyType != "cumulative" && tallyType != "plurality-at-large";
}

// prints the options selected on the ballot from the plaintext
void print_answer(
    stringstream & out,
    const Value & choice,
    const Value & question)
{
    if (!question.HasMember("title") || !question["title"].IsString())
    {
        out << "!!! Error [print-answer-title]: Invalid election format"
            << endl;
        throw runtime_error(out.str());
    }

    if (!choice.HasMember("plaintext") || !choice["plaintext"].IsString())
    {
        out << "!!! Error [print-answer-plaintext]:  Invalid election format"
            << endl;
        throw runtime_error(out.str());
    }

    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        out << "!!! Error [print-answer-answers]: Invalid election format"
            << endl;
        throw runtime_error(out.str());
    }

    Document ballot;
    try
    {
        ballot = parseBallot(question, choice["plaintext"].GetString());
    } catch (std::exception & error)
    {
        out << "!!! Error [decoding-ballot]: " << error.what() << endl;
        throw runtime_error(out.str());
    }

    out << endl << "Q: " << question["title"].GetString() << endl;
    out << "Ballot choices:" << endl;

    vector<Value *> answers =
        sortedAnswersVector(ballot["answers"], "selected");
    bool isInvalid = isInvalidBallot(ballot);
    bool isSortedQuestion = isSortedQuestionType(ballot);
    bool isBlank = true;
    size_t index = 1;

    for (const Value * answer: answers)
    {
        if ((*answer)["selected"].GetInt() == -1 ||
            AgoraAirgap::answerHasUrl(*answer, "invalidVoteFlag"))
        {
            continue;
        }
        isBlank = false;

        if (isSortedQuestion)
        {
            out << index << ". " << (*answer)["text"].GetString();
            index++;
        } else
        {
            out << " - " << (*answer)["text"].GetString();
        }

        if (AgoraAirgap::answerHasUrl(*answer, "isWriteIn"))
        {
            out << " (write-in)";
        }

        if (isInvalid)
        {
            out << " [invalid]";
        }
        out << endl;
    }

    if (isInvalid)
    {
        out << "- INVALID vote" << endl;
    } else if (isBlank)
    {
        out << "- BLANK vote" << endl;
    }
    out << endl;
}

void check_encrypted_answer(
    stringstream & out,
    const Value & choice,
    const Value & question,
    const Value & pubkey)
{
    if (!choice.HasMember("alpha") || !choice["alpha"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-alpha]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("beta") || !choice["beta"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-beta]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("plaintext") ||
        (!choice["plaintext"].IsString() && !choice["plaintext"].IsInt()))
    {
        out << "!!! Error [check-encrypted-answer-plaintext]: Invalid ballot "
               "format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("randomness") || !choice["randomness"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-randomness]: Invalid ballot "
               "format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("q") || !pubkey["q"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-q]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("p") || !pubkey["p"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-p]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("g") || !pubkey["g"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-g]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("y") || !pubkey["y"].IsString())
    {
        out << "!!! Error [check-encrypted-answer-y]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }

    mpz_class plain_vote, test;
    if (choice["plaintext"].IsString())
    {
        plain_vote.set_str(choice["plaintext"].GetString(), 10);
    } else if (choice["plaintext"].IsInt())
    {
        plain_vote = choice["plaintext"].GetInt();
    }

    mpz_class randomness;
    randomness = choice["randomness"].GetString();
    ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pubkey);
    ElGamal::Plaintext plaintext(plain_vote, pk, true);
    ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, randomness);

    mpz_class choiceAlpha, choiceBeta;
    choiceAlpha = choice["alpha"].GetString();
    choiceBeta = choice["beta"].GetString();

    out << "> verifying encrypted question: " << question["title"].GetString()
        << endl;

    if (0 == mpz_cmp(choiceAlpha.get_mpz_t(), ctext.alpha.get_mpz_t()) &&
        0 == mpz_cmp(choiceBeta.get_mpz_t(), ctext.beta.get_mpz_t()))
    {
        out << "> OK - Encrypted question verified" << endl;
    } else
    {
        out << "!!! Error [check-encrypted-answer-invalid]: INVALID - "
               "Encrypted question does not agree with plaintext "
               "vote"
            << endl;
        throw runtime_error(out.str());
    }
}

void check_ballot_hash(
    stringstream & out,
    const rapidjson::Document & originalBallot)
{
    rapidjson::Document ballot;
    ballot.CopyFrom(originalBallot, ballot.GetAllocator());

    if (!ballot.HasMember("ballot_hash") || !ballot["ballot_hash"].IsString())
    {
        out << "!!! Error [check-ballot-hash-ballot-hash]: Invalid ballot "
               "format"
            << endl;
        throw runtime_error(out.str());
    }

    string expectedHash = ballot["ballot_hash"].GetString();
    string generatedHash = generateBallotHash(originalBallot);

    out << "> verifying generated ballot hash: " << generatedHash << endl;

    if (generatedHash == expectedHash)
    {
        out << "> OK - hash verified" << endl;
    } else
    {
        out << "!!! Error [check-ballot-hash-invalid]: Invalid hash: "
            << generatedHash
            << " does not match expected hash: " << expectedHash << endl;
        throw runtime_error(out.str());
    }
}

void download_audit(
    stringstream & out,
    const string & auditable_ballot_path,
    const DownloadFunc & download_func)
{
    out << "> reading auditable ballot" << endl;
    download_audit_text(
        out, read_file(out, auditable_ballot_path), download_func);
}

void download_audit_text(
    stringstream & out,
    const string & auditable_ballot,
    const DownloadFunc & download_func)
{
    Document ballot, election, payload, pks;
    if (ballot.Parse(auditable_ballot.c_str()).HasParseError())
    {
        out << "!!! Error [download-audit-text-ballot-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.IsObject())
    {
        out << "!!! Error [download-audit-text-ballot]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString())
    {
        out << "!!! Error [download-audit-text-election-url]: Invalid ballot "
               "format"
            << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Error [download-audit-text-choices]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }

    string election_url = ballot["election_url"].GetString();
    out << "> election data downloaded";

    string election_data = download_func(out, election_url);

    if (election.Parse(election_data.c_str()).HasParseError())
    {
        out << "!!! Error [download-audit-text-election-json]: Json format "
               "error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!election.HasMember("payload"))
    {
        out << "!!! Error [download-audit-text-payload]: Invalid election "
               "format: "
            << stringify(election) << endl;
        throw runtime_error(out.str());
    }

    string payloads;

    if (election["payload"]["configuration"].IsString())
    {
        payloads = election["payload"]["configuration"].GetString();
    } else if (election["payload"]["configuration"].IsObject())
    {
        Document payload_doc;
        payload_doc.SetObject();
        payload_doc.CopyFrom(
            election["payload"]["configuration"], payload_doc.GetAllocator());
        payloads = stringify(payload_doc);
    } else
    {
        out << "!!! Error [download-audit-text-configuration]: Invalid "
               "election format"
            << endl;
        throw runtime_error(out.str());
    }

    out << "> election data configuration (hash: " +
               sha256::hex_sha256(payloads)
        << ")" << endl;

    out << "> parsing... (" << payloads.length() << " characters)" << endl;
    if (payload.Parse(payloads.c_str()).HasParseError())
    {
        out << "!!! Error [download-audit-payload-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!election["payload"].HasMember("pks") ||
        !election["payload"]["pks"].IsString())
    {
        out << "!!! Error [download-audit-text-pks]: Invalid election format "
               "pks\n"
            << stringify(payload) << endl;
        throw runtime_error(out.str());
    }

    string pkss = election["payload"]["pks"].GetString();
    if (pks.Parse(pkss.c_str()).HasParseError())
    {
        out << "!!! Error [download-audit-pks-json]: Json format error" << endl;
        throw runtime_error(out.str());
    }

    const Value & choices = ballot["choices"];
    if (!pks.IsArray() || pks.Size() != ballot["choices"].Size())
    {
        out << "!!! Error [download-audit-text-pks-choices]: Invalid public "
               "keys format: "
            << pks.Size() << " != " << ballot["choices"].Size() << endl;
        throw runtime_error(out.str());
    }
    const Value & proofs = ballot["proofs"];
    if (!proofs.IsArray() || proofs.Size() != pks.Size())
    {
        out << "!!! Error [download-audit-text-proofs]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }

    if (!payload.HasMember("questions") || !payload["questions"].IsArray() ||
        payload["questions"].Size() != choices.Size())
    {
        out << "!!! Error [download-audit-text-questions]: Invalid election "
               "format questions"
            << endl;
        throw runtime_error(out.str());
    }
    out << "> please check that the showed options are the ones you chose:"
        << endl;
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        print_answer(out, choices[i], payload["questions"][i]);
    }
    // check encrypted choices with plaintext
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        check_encrypted_answer(
            out, choices[i], payload["questions"][i], pks[i]);
    }
    // check hash
    check_ballot_hash(out, ballot);
    out << "> --------------------\n> Audit PASSED" << endl;
}

void download(
    stringstream & out,
    const string & auditable_ballot_path,
    const string & election_path,
    const DownloadFunc & download_func)
{
    Document ballot, pubkeys, election;
    out << "> reading auditable ballot" << endl;
    if (ballot.Parse(read_file(out, auditable_ballot_path).c_str())
            .HasParseError())
    {
        out << "!!! Error [download-ballot-read-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.IsObject())
    {
        out << "!!! Error [download-ballot]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Error [download-choices]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString())
    {
        out << "!!! Error [download-election-url]: Invalid ballot format"
            << endl;
        throw runtime_error(out.str());
    }
    string election_url = ballot["election_url"].GetString();

    string election_data = download_func(out, election_url);
    out << "> election data downloaded (hash: " +
               sha256::hex_sha256(election_data) + ")"
        << endl;

    out << "> parsing... (" << election_data.length() << " characters)" << endl;
    if (election.Parse(election_data.c_str()).HasParseError())
    {
        out << "!!! Error [download-election-parse-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    out << "> saving files..." << endl;

    if (!save_file(out, election_path, election_data))
    {
        out << "!!! Error [download-save]: Error writing to election data "
               "file " +
                   election_path
            << endl;
        throw runtime_error(out.str());
    }
}

void audit(
    stringstream & out,
    const string & auditable_ballot_path,
    const string & election_path)
{
    Document ballot, election, payload, pks;
    out << "> reading auditable ballot" << endl;
    string auditableBallot = read_file(out, auditable_ballot_path);
    if (ballot.Parse(auditableBallot.c_str()).HasParseError())
    {
        out << "!!! Error [audit-ballot-parse-json]: Json format error" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.IsObject())
    {
        out << "!!! Error [audit-ballot]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString())
    {
        out << "!!! Error [audit-election-url]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Error [audit-choices]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    string election_data = read_file(out, election_path);
    out << "> election data loaded (hash: " +
               sha256::hex_sha256(election_data) + ")"
        << endl;

    out << "> parsing... (" << election_data.length() << " characters)" << endl;

    if (election.Parse(election_data.c_str()).HasParseError())
    {
        out << "!!! Error [audit-ballot-parse-election-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!election.HasMember("payload"))
    {
        out << "!!! Error [audit-payload]: Invalid election format: "
            << stringify(election) << endl;
        throw runtime_error(out.str());
    }

    if (!election["payload"].HasMember("configuration"))
    {
        out << "!!! Error [audit-configuration]: Invalid election format"
            << endl;
        throw runtime_error(out.str());
    }

    string payloads;

    if (election["payload"]["configuration"].IsString())
    {
        payloads = election["payload"]["configuration"].GetString();
    } else if (election["payload"]["configuration"].IsObject())
    {
        Document payload_doc;
        payload_doc.SetObject();
        payload_doc.CopyFrom(
            election["payload"]["configuration"], payload_doc.GetAllocator());
        payloads = stringify(payload_doc);
    } else
    {
        out << "!!! Error [audit-configuration2]: Invalid election format"
            << endl;
        throw runtime_error(out.str());
    }

    out << "> election data configuration (hash: " +
               sha256::hex_sha256(payloads)
        << ")" << endl;

    out << "> parsing... (" << payloads.length() << " characters)" << endl;
    if (payload.Parse(payloads.c_str()).HasParseError())
    {
        out << "!!! Error [audit-configuration-parse-json]: Json format error"
            << endl;
        throw runtime_error(out.str());
    }

    if (!election["payload"].HasMember("pks") || election["payload"].IsString())
    {
        out << "!!! Error [audit-pks]: Invalid election format pks\n"
            << stringify(payload) << endl;
        throw runtime_error(out.str());
    }

    string pkss = election["payload"]["pks"].GetString();
    if (pks.Parse(pkss.c_str()).HasParseError())
    {
        out << "!!! Error [audit-pks-parse-json]: Json format error" << endl;
        throw runtime_error(out.str());
    }

    const Value & choices = ballot["choices"];
    if (!pks.IsArray() || pks.Size() != ballot["choices"].Size())
    {
        out << "!!! Error [audit-choices]: Invalid public keys format: "
            << pks.Size() << " != " << ballot["choices"].Size() << endl;
        throw runtime_error(out.str());
    }
    const Value & proofs = ballot["proofs"];
    if (!proofs.IsArray() || proofs.Size() != pks.Size())
    {
        out << "!!! Error [audit-proofs]: Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!payload.HasMember("questions") || !payload["questions"].IsArray() ||
        payload["questions"].Size() != choices.Size())
    {
        out << "!!! Error [audit-questions]: Invalid election format questions"
            << endl;
        throw runtime_error(out.str());
    }
    out << "> please check that the showed options are the ones you chose:"
        << endl;
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        print_answer(out, choices[i], payload["questions"][i]);
    }
    // check encrypted choices with plaintext
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        check_encrypted_answer(
            out, choices[i], payload["questions"][i], pks[i]);
    }
    // check hash
    check_ballot_hash(out, ballot);
    out << "> --------------------\n> Audit PASSED" << endl;
}

}  // namespace AgoraAirgap
