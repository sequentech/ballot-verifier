/*
 * Encrypt.cpp
 *
 *  Created on: November 19, 2014
 *      Authors:
 *    Eduardo Robles edulix at gmail dot com
 *    Félix Robles felrobelv at gmail dot com
 * Based on Eduardo Robles's agora-api:
 * https://github.com/agoravoting/agora-api
 */
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
#include <agora-airgap/Random.h>
#include <agora-airgap/encrypt.h>
#include <curl/curl.h>

using namespace std;
using namespace rapidjson;

namespace AgoraAirgap {

string get_date()
{
    time_t now = time(0);
    tm * gmtm = gmtime(&now);
    stringstream ss;  //"2014-11-24T19:47:13+00:00"
    ss << (1900 + gmtm->tm_year) << "-" << gmtm->tm_mon << "-" << gmtm->tm_mday
       << "T" << gmtm->tm_hour << ":" << gmtm->tm_min << ":" << gmtm->tm_sec
       << "+00:00";
    return ss.str();
}

string stringify(const Document & d)
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
        out << "!!! error opening file: " << path << endl;
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
    if (f.good())
    {
        f << text;
        f.close();
        return true;
    } else
    {
        f.close();
    }
    return false;
}

string encrypt_answer(
    stringstream & out, const Value & pk_json, const mpz_class & plain_vote)
{
    ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json);
    ElGamal::Plaintext plaintext(plain_vote, pk, true);
    mpz_class randomness = Random::getRandomIntegerRange(pk.q);
    out << "> plaintext = " << plain_vote.get_str(16)
        << "\n> randomness = " << randomness.get_str(16) << endl;
    ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, randomness);
    ElGamal::Fiatshamir_dlog_challenge_generator
        fiatshamir_dlog_challenge_generator;
    ElGamal::DLogProof proof = plaintext.proveKnowledge(
        ctext.alpha, randomness, fiatshamir_dlog_challenge_generator);

    bool verified =
        ctext.verifyPlaintextProof(proof, fiatshamir_dlog_challenge_generator);

    Document enc_answer;
    stringstream ss;
    ss << "{\"alpha\":\"" << ctext.alpha.get_str(10) << "\","
       << "\"beta\":\"" << ctext.beta.get_str(10) << "\","
       << "\"commitment\":" << proof.commitment.toJSON() << ","     // arr,t, s
       << "\"response\":\"" << proof.response.get_str(10) << "\","  //
       << "\"challenge\":\"" << proof.challenge.get_str(10) << "\"}";  //

    out << "> Node: proof verified = " << (verified ? "true" : "false") << endl;

    enc_answer.Parse(ss.str().c_str());

    return stringify(enc_answer);
}

// See examples:
// http://www.thomaswhitton.com/blog/2013/06/28/json-c-plus-plus-examples/

void encrypt_ballot(
    stringstream & out, const string & votes_path, const string & pk_path,
    const string & ballot_path)
{
    try
    {
        Document pk, ballots, votes;

        out << "> reading public keys" << endl;
        pk.Parse(read_file(out, pk_path).c_str());

        out << "> reading plaintext ballot" << endl;
        votes.Parse(read_file(out, votes_path).c_str());

        out << "> generating encrypted ballot" << endl;

        assert(votes.IsArray());

        const Value & votesArray = votes;

        assert(votesArray.IsArray());
        ballots.SetArray();
        string squestion;

        stringstream ssballot;
        ssballot << "[\n";
        for (SizeType i = 0; i < votesArray.Size(); i++)
        {
            out << "> question " << i << endl;
            mpz_class plain_vote;
            if (votesArray[i].IsString())
            {
                plain_vote = votesArray[i].GetString();
            } else if (votesArray[i].IsInt())
            {
                plain_vote = votesArray[i].GetInt();
            }

            Document ballot;
            ballot.SetObject();
            // stringstream ssballot;

            mpz_class bits, rand;
            bits = "160";
            rand = Random::getRandomIntegerBits(bits);
            string date, answ;
            date = get_date();
            answ = encrypt_answer(out, pk[0], plain_vote);
            if (i != 0)
            {
                ssballot << ",\n";
            }

            stringstream oneballot;
            oneballot << "\"is_vote_secret\":true,\"action\":\"vote\","
                      << "\"issue_date\":\"" << date << "\","
                      << "\"unique_randomness\":\"" << rand.get_str(16) << "\","
                      << "\"question0\":" << answ << "";

            ssballot << "{" << oneballot.str() << "}";
        }

        ssballot << "\n]";
        ballots.Parse(ssballot.str().c_str());
        // cout << "\n------------------\n" << stringify(ballots)<< endl;

        out << "> saving encrypted ballot to file..." << endl;
        if (!save_file(out, ballot_path, stringify(ballots)))
        {
            out << "!!! Error saving encrypted ballot to file path " +
                       ballot_path
                << endl;
        }
    } catch (...)
    {
        throw runtime_error(out.str());
    }
}

static size_t WriteCallback(
    void * contents, size_t size, size_t nmemb, void * userp)
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
        out << "curl doesn't work" << std::endl;
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
        out << "curl_easy_perform() failed:" << curl_easy_strerror(res) << endl;
        throw runtime_error(out.str());
    }

    curl_easy_cleanup(curl);
    return read_buffer;
}

// repeat n times string s
string repeat_string(const string & s, unsigned int n)
{
    stringstream out;
    while (n--)
        out << s;
    return out.str();
}

template <typename T>
string to_string(T i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

int to_int(string i)
{
    istringstream buffer(i);
    int value;
    buffer >> value;
    return value;
}

vector<int> split_choices(string choices, const Value & question)
{
    SizeType size = question["answers"].Size() + 2;
    int tabsize = to_string(size).length();
    vector<int> choicesV;
    // choices = repeat_string( string("0"),(choices.length() % tabsize) ) +
    // choices;
    for (int i = 0; i < choices.length() / tabsize; i++)
    {
        int choice = to_int(choices.substr(i * tabsize, tabsize)) - 1;
        choicesV.push_back(choice);
    }
    return choicesV;
}

const Value & find_value(
    stringstream & out, const Value & arr, string field, string value)
{
    for (SizeType i = 0; i < arr.Size(); i++)
    {
        const Value & el = arr[i];
        if (el.IsObject() && el.HasMember(field.c_str()))
        {
            if (el[field.c_str()].IsString() &&
                string(el[field.c_str()].GetString()).compare(value) == 0)
            {
                return el;
            } else if (
                el[field.c_str()].IsInt() &&
                to_string(el[field.c_str()].GetInt()).compare(value) == 0)
            {
                return el;
            }
        }
    }
    out << "value not found, exiting.." << endl;
    throw runtime_error(out.str());
}

// prints the options selected on the ballot from the plaintext
bool print_answer(
    stringstream & out, const Value & choice, const Value & question,
    const Value & pubkey)
{
    if (!question.HasMember("title") || !question["title"].IsString())
    {
        out << "!!! Invalid election format" << endl;
        throw runtime_error(out.str());
    }

    if (!choice.HasMember("plaintext") || !choice["plaintext"].IsString())
    {
        out << "!!! Invalid election format" << endl;
        throw runtime_error(out.str());
    }

    if (!question.HasMember("answers") || !question["answers"].IsArray())
    {
        out << "!!! Invalid election format" << endl;
        throw runtime_error(out.str());
    }

    out << "Q: " << question["title"].GetString() << endl;
    SizeType size = question["answers"].Size() + 2;
    vector<int> choices =
        split_choices(choice["plaintext"].GetString(), question);
    out << "user answers:" << endl;
    for (int i = 0; i < choices.size(); i++)
    {
        if (choices.at(i) >= 0 && choices.at(i) < size - 1)
        {
            out << "choice " << to_string(choices.at(i)) << endl;
            out << " - "
                << find_value(
                       out, question["answers"], "id",
                       to_string(choices.at(i)))["text"]
                       .GetString()
                << endl;
        } else if (choices.at(i) == size - 1)
        {
            out << "choice " << to_string(choices.at(i)) << endl;
            out << "- BLANK vote" << endl;
        } else
        {
            out << "choice " << to_string(choices.at(i)) << endl;
            out << "- NULL vote" << endl;
        }
    }
}

void check_encrypted_answer(
    stringstream & out, const Value & choice, const Value & question,
    const Value & pubkey)
{
    if (!choice.HasMember("alpha") || !choice["alpha"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("beta") || !choice["beta"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("plaintext") ||
        (!choice["plaintext"].IsString() && !choice["plaintext"].IsInt()))
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!choice.HasMember("randomness") || !choice["randomness"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("q") || !pubkey["q"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("p") || !pubkey["p"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("g") || !pubkey["g"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    if (!pubkey.HasMember("y") || !pubkey["y"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
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
        out << "!!! INVALID - Encrypted question does not agree with plaintext "
               "vote"
            << endl;
        throw runtime_error(out.str());
    }
}

void check_ballot_hash(stringstream & out, rapidjson::Document & ballot)
{
    if (!ballot.HasMember("ballot_hash") || !ballot["ballot_hash"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    string ballot_hash = ballot["ballot_hash"].GetString();

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

    string compare_hash = sha256::hex_sha256(ballotss.str());

    out << "> verifying ballot hash: " << compare_hash << endl;
    if (compare_hash.compare(ballot_hash) == 0)
    {
        out << "> OK - hash verified" << endl;
    } else
    {
        out << "!!! Invalid hash: " + compare_hash << endl;
        throw runtime_error(out.str());
    }
}

void download_audit(stringstream & out, const string & auditable_ballot_path)
{
    out << "> reading auditable ballot" << endl;
    download_audit_text(out, read_file(out, auditable_ballot_path));
}

void download_audit_text(stringstream & out, const string & auditable_ballot)
{
    Document ballot, election, payload, pks;
    ballot.Parse(auditable_ballot.c_str());

    if (!ballot.IsObject())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    string election_url = ballot["election_url"].GetString();
    out << "> election data downloaded";

    string election_data = download_url(out, election_url);

    election.Parse(election_data.c_str());

    if (!election.HasMember("payload"))
    {
        out << "!!! Invalid election format: " << stringify(election) << endl;
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
        out << "!!! Invalid election format" << endl;
        throw runtime_error(out.str());
    }

    out << "> election data configuration hash: " + sha256::hex_sha256(payloads)
        << endl;

    out << "> parsing..." << endl;
    payload.Parse(payloads.c_str());

    if (!election["payload"].HasMember("pks") ||
        !election["payload"]["pks"].IsString())
    {
        out << "!!! Invalid election format pks\n"
            << stringify(payload) << endl;
        throw runtime_error(out.str());
    }

    string pkss = election["payload"]["pks"].GetString();
    pks.Parse(pkss.c_str());

    const Value & choices = ballot["choices"];
    if (!pks.IsArray() || pks.Size() != ballot["choices"].Size())
    {
        out << "!!! Invalid public keys format: " << pks.Size()
            << " != " << ballot["choices"].Size() << endl;
        throw runtime_error(out.str());
    }
    const Value & proofs = ballot["proofs"];
    if (!proofs.IsArray() || proofs.Size() != pks.Size())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!payload.HasMember("questions") || !payload["questions"].IsArray() ||
        payload["questions"].Size() != choices.Size())
    {
        out << "!!! Invalid election format questions" << endl;
        throw runtime_error(out.str());
    }
    out << "> please check that the showed options are the ones you chose:"
        << endl;
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        print_answer(out, choices[i], payload["questions"][i], pks[i]);
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
    stringstream & out, const string & auditable_ballot_path,
    const string & election_path)
{
    Document ballot, pubkeys, election;
    out << "> reading auditable ballot" << endl;
    ballot.Parse(read_file(out, auditable_ballot_path).c_str());

    if (!ballot.IsObject())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }
    string election_url = ballot["election_url"].GetString();

    string election_data = download_url(out, election_url);
    out << "> election data downloaded (hash: " +
               sha256::hex_sha256(election_data) + ")"
        << endl;

    out << "> parsing..." << endl;
    election.Parse(election_data.c_str());

    out << "> saving files..." << endl;

    if (!save_file(out, election_path, election_data))
    {
        out << "!!! Error writing to election data file " + election_path
            << endl;
        throw runtime_error(out.str());
    }
}

void audit(
    stringstream & out, const string & auditable_ballot_path,
    const string & election_path)
{
    Document ballot, election, payload, pks;
    out << "> reading auditable ballot" << endl;
    ballot.Parse(read_file(out, auditable_ballot_path).c_str());

    if (!ballot.IsObject())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!ballot.HasMember("choices") || !ballot["choices"].IsArray())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    string election_data = read_file(out, election_path);
    out << "> election data loaded (hash: " +
               sha256::hex_sha256(election_data) + ")"
        << endl;

    out << "> parsing..." << endl;

    election.Parse(election_data.c_str());

    if (!election.HasMember("payload"))
    {
        out << "!!! Invalid election format: " << stringify(election) << endl;
        throw runtime_error(out.str());
    }

    if (!election["payload"].HasMember("configuration") ||
        !election["payload"]["configuration"].IsString())
    {
        out << "!!! Invalid election format" << endl;
        throw runtime_error(out.str());
    }

    string payloads = election["payload"]["configuration"].GetString();
    payload.Parse(payloads.c_str());

    if (!election["payload"].HasMember("pks") || election["payload"].IsString())
    {
        out << "!!! Invalid election format pks\n"
            << stringify(payload) << endl;
        throw runtime_error(out.str());
    }

    string pkss = election["payload"]["pks"].GetString();
    pks.Parse(pkss.c_str());

    const Value & choices = ballot["choices"];
    if (!pks.IsArray() || pks.Size() != ballot["choices"].Size())
    {
        out << "!!! Invalid public keys format: " << pks.Size()
            << " != " << ballot["choices"].Size() << endl;
        throw runtime_error(out.str());
    }
    const Value & proofs = ballot["proofs"];
    if (!proofs.IsArray() || proofs.Size() != pks.Size())
    {
        out << "!!! Invalid ballot format" << endl;
        throw runtime_error(out.str());
    }

    if (!payload.HasMember("questions") || !payload["questions"].IsArray() ||
        payload["questions"].Size() != choices.Size())
    {
        out << "!!! Invalid election format questions" << endl;
        throw runtime_error(out.str());
    }
    out << "> please check that the showed options are the ones you chose:"
        << endl;
    for (SizeType i = 0; i < choices.Size(); ++i)
    {
        print_answer(out, choices[i], payload["questions"][i], pks[i]);
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
