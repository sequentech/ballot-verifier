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
#include "sha256.h"

using namespace std;

class ElGamal
{
public:

	class PlaintextCommitment;
	class DLogProof;
	class Challenge_Generator;
	class Params;
	class SecretKey;
	class PublicKey;

	class Params {
	public:
		mpz_t p, q, g;
		Params();
		Params(const mpz_t &p, const mpz_t &q, const mpz_t &g);
		SecretKey generate();
	};


	class PublicKey	{
	public:
		mpz_t p, q, g, y;
		PublicKey(const mpz_t &p, const mpz_t &q, const mpz_t &g, const mpz_t &y);
		PublicKey(const ElGamal::PublicKey &pk);
		PublicKey();
		static PublicKey fromJSONObject(const string &pk_json);
	};

	class SecretKey {
	public:
		mpz_t x;
		ElGamal::PublicKey pk;
		SecretKey(const mpz_t &x, const PublicKey &pk);
	};

	class Challenge_Generator {
	public:
		virtual void generator(mpz_t &out, const ElGamal::PlaintextCommitment &commitment) const{}
	};

	class Fiatshamir_dlog_challenge_generator: public Challenge_Generator {
	public:
		void generator(mpz_t &out, const ElGamal::PlaintextCommitment &commitment) const;
	};

	class Plaintext	{
	public:
		PublicKey pk;
		Plaintext(const mpz_t &m, const PublicKey &pk, const bool &encode_m);
		void getPlaintext(mpz_t &res) const;
		void getM(mpz_t &res) const;
		ElGamal::DLogProof proveKnowledge(const mpz_t &alpha, const mpz_t &randomness, const Challenge_Generator &challenge_generator) const;
	private:
		mpz_t m;
	};

	class PlaintextCommitment {
	public:
		mpz_t a, alpha;
		PlaintextCommitment();
		PlaintextCommitment(const mpz_t &alpha, const mpz_t &a);
		string toString() const;
	};

	class DLogProof	{
	public:
		ElGamal::PlaintextCommitment commitment;
		mpz_t challenge,response;
		DLogProof();
		DLogProof(const ElGamal::PlaintextCommitment &commitment , const mpz_t &challenge, const mpz_t &response);
	};


	class Ciphertext {
	public:
			PublicKey pk;
			mpz_t alpha, beta;
			Ciphertext();
			Ciphertext(const mpz_t &alpha, const mpz_t &beta, const PublicKey &pk);
			string toString();

	};

	static ElGamal::Ciphertext encrypt(const PublicKey &pk, const Plaintext &plaintext, const mpz_t &r);
  	static ElGamal::Plaintext decrypt(const SecretKey &sk, const Ciphertext &ciphertext);
};



#endif /* ELGAMAL_H_ */

