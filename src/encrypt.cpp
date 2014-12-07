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
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <set>

#include <gmpxx.h>
#include <curl/curl.h>
#include "encrypt.h"
#include "Random.h"
#include "ElGamal.h"

using namespace std;
using namespace rapidjson;

string get_date()
{
  time_t now = time(0);
  tm * gmtm = gmtime(&now);
  stringstream ss; //"2014-11-24T19:47:13+00:00"
  ss << (1900+gmtm->tm_year) << "-" << gmtm->tm_mon << "-" << gmtm->tm_mday << "T" <<
    gmtm->tm_hour << ":" << gmtm->tm_min << ":" << gmtm->tm_sec << "+00:00";
  return ss.str();
}

string stringify(const Document & d)
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  d.Accept(writer);
  return buffer.GetString();
}

string read_file(const string & path)
{
  ifstream t(path.c_str());
  string str;

  t.seekg(0, std::ios::end);   
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign( (std::istreambuf_iterator<char>(t)),
      std::istreambuf_iterator<char>());
  return str;
}

bool save_file(const string & path, const string & text)
{
  cout << "> writing to file " + path << endl;
  ofstream f(path.c_str());
  if(f.good())
  {
    f << text;
    f.close();
    return true;
  }
  else
  {
    f.close();
  }
  return false;
}

string encrypt_answer(const Value & pk_json, const mpz_class & plain_vote)
{
  ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json);
  ElGamal::Plaintext plaintext(plain_vote, pk, true);
  mpz_class randomness = Random::getRandomIntegerRange(pk.q);
  cout << "> plaintext = " << plain_vote.get_str(16)  << "\n> randomness = " << randomness.get_str(16) << endl;
  ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, randomness);
  ElGamal::Fiatshamir_dlog_challenge_generator fiatshamir_dlog_challenge_generator;
  ElGamal::DLogProof proof = plaintext.proveKnowledge(ctext.alpha, randomness, fiatshamir_dlog_challenge_generator);
  
  bool verified = ctext.verifyPlaintextProof(proof, fiatshamir_dlog_challenge_generator);
  
  Document enc_answer;
  stringstream ss;
  ss  << "{\"alpha\":\"" << ctext.alpha.get_str(10)   << "\"," 
    << "\"beta\":\""  << ctext.beta.get_str(10)   << "\"," 
    << "\"commitment\":"  << proof.commitment.toJSON()  << "," //arr,t, s
    << "\"response\":\""  << proof.response.get_str(10) << "\"," //
    << "\"challenge\":\""  << proof.challenge.get_str(10) << "\"}";//
  
  cout << "> Node: proof verified = " << (verified? "true" : "false") << endl;
  
  enc_answer.Parse(ss.str().c_str());
  
  return stringify(enc_answer);
}

//See examples: http://www.thomaswhitton.com/blog/2013/06/28/json-c-plus-plus-examples/

void encrypt_ballot(const string & votes_path, const string & pk_path, const string & ballot_path)
{  
  Document pk, ballots, votes;
  
  cout << "> reading public keys" << endl;
  pk.Parse( read_file(pk_path).c_str() );
  
  cout << "> reading plaintext ballot" << endl;
  votes.Parse( read_file(votes_path).c_str() );
  
  cout << "> generating encrypted ballot" << endl;
  
  assert(votes.IsArray());
  
  const Value& votesArray = votes;
  
  assert(votesArray.IsArray());
  ballots.SetArray(); 
  string squestion;
  
  stringstream ssballot;
  ssballot << "[\n";
  for(SizeType i = 0; i < votesArray.Size(); i++)
  {
    cout << "> question " << i << endl;
    mpz_class plain_vote;
    if( votesArray[i].IsString() )
    {
      plain_vote = votesArray[i].GetString();
    } 
    else if( votesArray[i].IsInt() ) 
    {
      plain_vote = votesArray[i].GetInt();
    } 
    
    Document ballot;
    ballot.SetObject(); 
    //stringstream ssballot;
    
    
    mpz_class bits, rand;
    bits = "160";
    rand = Random::getRandomIntegerBits(bits);
    string date, answ;
    date = get_date();
    answ = encrypt_answer(pk[0], plain_vote);
    if(i != 0)
    {
      ssballot << ",\n";
    }     
    
    stringstream oneballot;
    oneballot<< "\"is_vote_secret\":true,\"action\":\"vote\","
      << "\"issue_date\":\""    << date     << "\","
      << "\"unique_randomness\":\""   << rand.get_str(16) << "\","
      << "\"question0\":"   << answ     << "";
        
    ssballot << "{" << oneballot.str() << "}";
  }
  
  ssballot << "\n]";
  ballots.Parse(ssballot.str().c_str() );
  //cout << "\n------------------\n" << stringify(ballots)<< endl;
  
  cout << "> saving encrypted ballot to file..."  << endl;
  if( !save_file(ballot_path, stringify(ballots)) )
  {
    cout << "!!! Error saving encrypted ballot to file path " + ballot_path << endl;
  }
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

string download_url(const string& url)
{

  CURL *curl;
  CURLcode res;
  std::string read_buffer;

  curl = curl_easy_init();
  if(!curl) {
      std::cout << "curl doesn't work" << std::endl;
      exit(1);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
  res = curl_easy_perform(curl);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    exit(1);
  }

  curl_easy_cleanup(curl);
  return read_buffer;
}

//repeat n times string s
string repeat_string(const string& s, unsigned int n) 
{
    stringstream out;
    while (n--)
        out << s;
    return out.str();
}

template<typename T>
string to_string(T i) {
  stringstream ss;
  ss << i;
  return ss.str();
}

int to_int(string i) {
  istringstream buffer(i);
  int value;
  buffer >> value;
  return value;
}

vector<int> split_choices(string choices, const Value& question) {
  SizeType size = question["answers"].Size();
  int tabsize = to_string(size).length();
  vector<int> choicesV;
  choices = repeat_string( string("0"),(choices.length() % tabsize) ) + choices;
  for(int i = 0; i < choices.length() / tabsize; i++) {
    int choice = to_int(choices.substr(i*tabsize, tabsize));
    choicesV.push_back(choice);
  }
  return choicesV;
}

const Value& find_value(const Value& arr, string field, string value) {
  for (SizeType i = 0; i < arr.Size(); i++) {
    const Value& el = arr[i];
    if (el.IsObject() && el.HasMember(field.c_str())) {
      if(el[field.c_str()].IsString() && string(el[field.c_str()].GetString()).compare(value) == 0)
      {
          return el;
      }
      else if ( el[field.c_str()].IsInt() && to_string( el[field.c_str()].GetInt()  ).compare(value) == 0 )
      {
          return el;
      }
    }
  }
  cout << "value not found, exiting.." << endl;
  exit(1);
}

//prints the options selected on the ballot from the plaintext
bool print_answer(const Value& choice, const Value& question, const Value& pubkey) 
{
  
  if(!question.HasMember("question") || !question["question"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
  exit(1);
  }

  if(!choice.HasMember("plaintext") || !choice["plaintext"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if(!question.HasMember("answers") || !question["answers"].IsArray()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  cout << "Q: " << question["question"].GetString() << endl;
  vector<int> choices = split_choices(choice["plaintext"].GetString(), question);  
  cout << "user answers:" << endl;
  for (int i = 0; i < choices.size(); i++) {
    cout << " - " << find_value( question["answers"], "id", to_string( choices.at(i) ) )["value"].GetString() << endl;
  }
}

void check_encrypted_answer(const Value & choice, const Value & question, 
                            const Value & pubkey)
{  
  if(!choice.HasMember("alpha") || !choice["alpha"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!choice.HasMember("beta") || !choice["beta"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!choice.HasMember("plaintext") || ( !choice["plaintext"].IsString()  && !choice["plaintext"].IsInt() ) ){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!choice.HasMember("randomness") || !choice["randomness"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!pubkey.HasMember("q") || !pubkey["q"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!pubkey.HasMember("p") || !pubkey["p"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!pubkey.HasMember("g") || !pubkey["g"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  if(!pubkey.HasMember("y") || !pubkey["y"].IsString()){
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  mpz_class plain_vote;
  if( choice["plaintext"].IsString() )
  {
    plain_vote = choice["plaintext"].GetString();
  } 
  else if( choice["plaintext"].IsInt() ) 
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
   
  cout << "> verifying encrypted question: " << question["question"].GetString() << endl;

  if(0 == mpz_cmp(choiceAlpha.get_mpz_t(), ctext.alpha.get_mpz_t())  
    && 0 == mpz_cmp(choiceBeta.get_mpz_t(), ctext.beta.get_mpz_t())) 
  {
    cout << "> OK - Encrypted question verified" << endl;
  }
  else 
  {
    cout << "!!! INVALID - Encrypted question does not agree with plaintext vote" << endl;
    exit(1);
  }
}

void check_ballot_hash(rapidjson::Document & ballot)
{
  if (!ballot.HasMember("ballot_hash") || !ballot["ballot_hash"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  string ballot_hash = ballot["ballot_hash"].GetString();
  
  ballot.RemoveMember("pubkeys_url");
  ballot.RemoveMember("election_url");
  ballot.RemoveMember("ballot_hash");
  for (SizeType i = 0; i < ballot["choices"].Size(); ++i) {
    ballot["choices"][i].RemoveMember("plaintext");
    ballot["choices"][i].RemoveMember("randomness");
  }
  
  stringstream ballotss;
  string partial;
  
  Document choices_doc;
  choices_doc.SetObject();
  choices_doc.AddMember("choices", ballot["choices"], choices_doc.GetAllocator());
  partial = stringify(choices_doc);
  partial = partial.substr(1, partial.size()-2);
  
  ballotss << "{\"a\":\"" << ballot["a"].GetString() << "\"," << partial << ",";
  
  Document election_hash_doc;
  election_hash_doc.SetObject();
  election_hash_doc.AddMember("election_hash", ballot["election_hash"], election_hash_doc.GetAllocator());
  partial = stringify(election_hash_doc);
  partial = partial.substr(1, partial.size()-2);
  
  ballotss << partial  << ",\"issue_date\":\"" << ballot["issue_date"].GetString() << "\",";
  
  Document proofs_doc;
  proofs_doc.SetObject();
  proofs_doc.AddMember("proofs", ballot["proofs"], proofs_doc.GetAllocator());
  partial = stringify(proofs_doc);
  partial = partial.substr(1, partial.size()-2);
  
  ballotss << partial  << "}";
  
  string compare_hash = hex_sha256(ballotss.str());
  
  cout << "> verifying ballot hash: " << compare_hash<< endl;
  if (compare_hash.compare(ballot_hash) == 0) {
    cout << "> OK - hash verified" << endl;
  } else {
    cout << "!!! Invalid hash: " + compare_hash << endl;
    exit(1);
  }
}

void download_audit(const string& auditable_ballot_path)
{
  Document ballot, pubkeys, election;
  cout << "> reading auditable ballot" << endl;
  ballot.Parse( read_file(auditable_ballot_path).c_str() );

  if (!ballot.IsObject()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("a") || !ballot["a"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("pubkeys_url") || !ballot["pubkeys_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  string election_url = ballot["election_url"].GetString();
  string pubkeys_url = ballot["pubkeys_url"].GetString();

  string election_data = download_url(election_url);
  cout << "> election data downloaded (hash: " + hex_sha256(election_data) + ")" << endl;
  string pubkeys_data = download_url(pubkeys_url);
  cout << "> public keys downloaded (hash: " + hex_sha256(pubkeys_data) + ")" << endl;
  
  cout << "> parsing..."  << endl;
  
  pubkeys.Parse( pubkeys_data.c_str() );
  election.Parse( election_data.c_str() );

  const Value& choices = ballot["choices"];
  if (!pubkeys.IsArray() || pubkeys.Size() != ballot["choices"].Size()) {
    cout << "!!! Invalid public keys format" << endl;
    exit(1);
  }
  const Value& proofs = ballot["proofs"];
  if (!proofs.IsArray() || proofs.Size() != pubkeys.Size()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!election.HasMember("questions_data") || !election["questions_data"].IsArray() || election["questions_data"].Size() != choices.Size()) {
    cout << "!!! Invalid election format" << endl;
    exit(1);
  }
  cout << "> please check that the showed options are the ones you chose:" << endl;
  for (SizeType i = 0; i < choices.Size(); ++i) {
    print_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  //check encrypted choices with plaintext
  for (SizeType i = 0; i < choices.Size(); ++i) {
    check_encrypted_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  //check hash
  check_ballot_hash(ballot);
  cout << "> --------------------\n> Audit PASSED" << endl;
}

void download(const string & auditable_ballot_path, const string & pk_path, const string &  voting_options_path)
{
  Document ballot, pubkeys, election;
  cout << "> reading auditable ballot" << endl;
  ballot.Parse( read_file(auditable_ballot_path).c_str() );

  if (!ballot.IsObject()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("a") || !ballot["a"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("pubkeys_url") || !ballot["pubkeys_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  string pubkeys_url = ballot["pubkeys_url"].GetString();
  string election_url = ballot["election_url"].GetString();

  string pubkeys_data = download_url(pubkeys_url);
  cout << "> public keys downloaded (hash: " + hex_sha256(pubkeys_data) + ")" << endl;
  string election_data = download_url(election_url);
  cout << "> election data downloaded (hash: " + hex_sha256(election_data) + ")" << endl;
  
  cout << "> parsing..."  << endl;
  
  pubkeys.Parse( pubkeys_data.c_str() );
  election.Parse( election_data.c_str() );

  cout << "> saving files..."  << endl;
  if(!save_file(pk_path, pubkeys_data))
  {
    cout << "!!! Error writing to public keys file " + pk_path << endl;
    exit(1);
  }
  if(!save_file(voting_options_path, election_data))
  {
    cout << "!!! Error writing to election data file " + pk_path << endl;
    exit(1);
  }
}

void audit(const string & auditable_ballot_path, const string & pk_path, const string &  voting_options_path)
{
  Document ballot, pubkeys, election;
  cout << "> reading auditable ballot" << endl;
  ballot.Parse( read_file(auditable_ballot_path).c_str() );

  if (!ballot.IsObject()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("a") || !ballot["a"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  if (!ballot.HasMember("election_url") || !ballot["election_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("choices") || !ballot["choices"].IsArray()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!ballot.HasMember("pubkeys_url") || !ballot["pubkeys_url"].IsString()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }
  
  cout << "> reading public keys" << endl;
  string pubkeys_data = read_file(pk_path).c_str();
  cout << "> public keys loaded (hash: " + hex_sha256(pubkeys_data) + ")" << endl;
  

  cout << "> reading election data" << endl;
  string election_data = read_file(voting_options_path).c_str();
  cout << "> election data loaded (hash: " + hex_sha256(election_data) + ")" << endl;
  
  cout << "> parsing..."  << endl;
  
  pubkeys.Parse( pubkeys_data.c_str() );
  election.Parse( election_data.c_str() );

  const Value& choices = ballot["choices"];
  if (!pubkeys.IsArray() || pubkeys.Size() != ballot["choices"].Size()) {
    cout << "!!! Invalid public keys format" << endl;
    exit(1);
  }
  const Value& proofs = ballot["proofs"];
  if (!proofs.IsArray() || proofs.Size() != pubkeys.Size()) {
    cout << "!!! Invalid ballot format" << endl;
    exit(1);
  }

  if (!election.HasMember("questions_data") || !election["questions_data"].IsArray() || election["questions_data"].Size() != choices.Size()) {
    cout << "!!! Invalid election format" << endl;
    exit(1);
  }
  cout << "> Please check that the showed options are the ones you chose:" << endl;
  for (SizeType i = 0; i < choices.Size(); ++i) {
    print_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  //check encrypted choices with plaintext
  for (SizeType i = 0; i < choices.Size(); ++i) {
    check_encrypted_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  //check hash
  check_ballot_hash(ballot);
  cout << "> --------------------\n> Audit PASSED" << endl;
}
