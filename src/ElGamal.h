/*
 * ElGamal.h
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */
#ifndef ELGAMAL_H_
#define ELGAMAL_H_

#include <gmp.h>
#include <string>
#include <iostream>

using namespace std;

class ElGamal
{
public:

	class PlaintextCommitment;
	class DLogProof;
	class PublicKey
	{
	public:
		mpz_t p, q, g, y;
		PublicKey(const mpz_t pp, const mpz_t qq, const mpz_t gg, const mpz_t yy);
		PublicKey(const ElGamal::PublicKey &pk);
		PublicKey();
		static PublicKey fromJSONObject(const string pk_json);
	};

	class Challenge_Generator
	{
	public:
		void generator(mpz_t out, const ElGamal::PlaintextCommitment commitment);
	};

	class Fiatshamir_dlog_challenge_generator: public Challenge_Generator
	{
	public:
		virtual void generator(mpz_t out, const ElGamal::PlaintextCommitment commitment);
	};


	class Plaintext
	{
	public:
		PublicKey pk;
		Plaintext(const mpz_t m, const PublicKey pk, const bool encode_m);
		void getPlaintext(mpz_t res) const;
		void getM(mpz_t res) const;
		ElGamal::DLogProof proveKnowledge(const mpz_t alpha, const mpz_t randomness, const Challenge_Generator challenge_generator);
	private:
		mpz_t m;
	};

	class PlaintextCommitment
	{
	public:
		mpz_t a, alpha;
		PlaintextCommitment();
		PlaintextCommitment(const mpz_t alpha, const mpz_t a);
	};

	class DLogProof
	{
		ElGamal::PlaintextCommitment commitment;
		mpz_t challenge,response;
	public:
		DLogProof();
		DLogProof(ElGamal::PlaintextCommitment commitment , mpz_t challenge, mpz_t response);
	};


	class Ciphertext
	{
	public:
			PublicKey pk;
			mpz_t alpha, beta;
			Ciphertext();
			Ciphertext(const mpz_t alpha, const mpz_t beta, const PublicKey pk);
			string toString();

	};

	static ElGamal::Ciphertext encrypt(const PublicKey pk, const Plaintext plaintext, const mpz_t r);
};



#endif /* ELGAMAL_H_ */

