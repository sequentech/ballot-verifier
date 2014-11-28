/*
 * Encrypt.cpp
 *
 *  Created on: November 19, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Based on Eduardo Robles's agora-api:
 * https://github.com/agoravoting/agora-api
 */
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

#include <gmpxx.h>
#include "encrypt.h"
#include "Random.h"
#include "ElGamal.h"
//#include "libjson.h"

using namespace std;
using namespace rapidjson;
//using namespace libjson;

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

string encryptAnswer(const Document & pk_json, const mpz_class & plain_vote)
{
	ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json[0]);
	ElGamal::Plaintext plaintext(plain_vote, pk, true);
	mpz_class randomness = Random::getRandomIntegerRange(pk.q);
	cout << "plaintext = " << plain_vote.get_str(16)  << ", randomness = " << randomness.get_str(16) << endl;
	ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, randomness);
	ElGamal::Fiatshamir_dlog_challenge_generator fiatshamir_dlog_challenge_generator;
	ElGamal::DLogProof proof = plaintext.proveKnowledge(ctext.alpha, randomness, fiatshamir_dlog_challenge_generator);
	
	bool verified = ctext.verifyPlaintextProof(proof, fiatshamir_dlog_challenge_generator);
	
	Document enc_answer;
	stringstream ss;
	ss	<< "{\"alpha\":\"" << ctext.alpha.get_str(10)		<< "\"," 
		<< "\"beta\":\""  << ctext.beta.get_str(10)		<< "\"," 
		<< "\"commitment\":"  << proof.commitment.toJSON()	<< "," //arr,t, s
		<< "\"response\":\""  << proof.response.get_str(10)	<< "\"," //
		<< "\"challenge\":\""  << proof.challenge.get_str(10)	<< "\"}";//
	
	//cout << "\n------\n" << ss.str() << "\n------" << endl;
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
		answ = encryptAnswer(pk, plain_vote);
		if(i != 0)
		{
			ssballot << ",\n";
		}			
		
		stringstream oneballot;
		oneballot<< "\"is_vote_secret\":true,\"action\":\"vote\","
			<< "\"issue_date\":\""		<< date			<< "\","
			<< "\"unique_randomness\":\"" 	<< rand.get_str(16)	<< "\","
			<< "\"question0\":"		<< answ			<< "";
				
		ssballot << "{" << oneballot.str() << "}";
	}
	
	ssballot << "\n]";
	ballots.Parse(ssballot.str().c_str() );
	cout << "\n------------------\n" << stringify(ballots)<< endl;
	
	/*JSONNode n(JSON_ARRAY);
	n.push_back(JSONNode("", ssballot.str() ));
	
	
	cout << "\n------------------\n" << n.write_formatted() << endl;*/
	
	
	return ret;
}
