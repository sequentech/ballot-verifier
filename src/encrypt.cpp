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

string readFile(const string & path)
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

string encryptAnswer(const Value & pk_json, const mpz_class & plain_vote)
{
  ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json);
  ElGamal::Plaintext plaintext(plain_vote, pk, true);
  mpz_class randomness = Random::getRandomIntegerRange(pk.q);
  cout << "plaintext = " << plain_vote.get_str(16)  << ", randomness = " << randomness.get_str(16) << endl;
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

string encrypt(const string & pk_path, const string & votes_path)
{
  string ret;
  
  Document pk, ballots, votes;
  
  cout << "> Node: reading pk" << endl;
  pk.Parse( readFile(pk_path).c_str() );
  
  votes.Parse( readFile(votes_path).c_str() );
  
  assert(votes.IsArray());
  
  const Value& votesArray = votes;
  
  assert(votesArray.IsArray());
  ballots.SetArray(); 
  string squestion;
  
  stringstream ssballot;
  ssballot << "[\n";
  for(SizeType i = 0; i < votesArray.Size(); i++)
  {
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
    answ = encryptAnswer(pk[0], plain_vote);
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
  cout << "\n------------------\n" << stringify(ballots)<< endl;
  
  return ret;
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
  cout << "User answers:" << endl;
  for (int i = 0; i < choices.size(); i++) {
    cout << " - " << find_value( question["answers"], "id", to_string( choices.at(i) ) )["value"].GetString() << endl;
  }
}

void check_encrypted_answer(const Value & choice, const Value & question, const Value & pubkey)
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
 
  cout << "> Verifying encrypted question: " << question["question"].GetString() << endl;

  if( 0 == mpz_cmp(choiceAlpha.get_mpz_t(), ctext.alpha.get_mpz_t())  
    && 0 == mpz_cmp(choiceBeta.get_mpz_t(), ctext.beta.get_mpz_t())) 
  {
    cout << "> OK - Encrypted question verified" << endl;
  }
  else 
  {
    cout << "!!! INVALID - Encrypted question does not agree with plaintext vote" << endl;
  }
}

string audit(const string& auditable_ballot_path)
{
  Document ballot, pubkeys, election;
  cout << "> Reading auditable ballot" << endl;
  ballot.Parse( readFile(auditable_ballot_path).c_str() );

  if (!ballot.IsObject()) {
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

  if (!election.HasMember("questions_data") || !election["questions_data"].IsArray() || election["questions_data"].Size() != choices.Size()) {
    cout << "!!! Invalid election format" << endl;
    exit(1);
  }
  cout << "> Please check that the showed options are the ones you chose:" << endl;
  for (SizeType i = 0; i < choices.Size(); i++) {
    print_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  //check encrypted choices with plaintext
  for (SizeType i = 0; i < choices.Size(); i++) {
    check_encrypted_answer(choices[i], election["questions_data"][i], pubkeys[i]);
  }
  
  return string("> --------------------\n> Audit PASSED");
}
