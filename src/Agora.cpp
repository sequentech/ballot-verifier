/*
 * Agora.cpp
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Agora Voting:
 * https://github.com/agoraciudadana/agora-ciudadana/blob/security/agora_site/static/js/agora/views/voting_booth.js
 */


#include "Agora.h"
#include "Random.h"

Agora::Encrypted_answer::Encrypted_answer()
{
	mpz_init(alpha);
	mpz_init(beta);
	mpz_init(challenge);
	mpz_init(response);
}

Agora::Encrypted_answer::Encrypted_answer(const mpz_t &alpha, const mpz_t &beta, const ElGamal::PlaintextCommitment &commitment,
				const mpz_t &challenge,const mpz_t &response)
{
	this->commitment = commitment;
	mpz_init_set(this->alpha, alpha);
	mpz_init_set(this->beta, beta);
	mpz_init_set(this->challenge, challenge);
	mpz_init_set(this->response, response);
}

Agora::Encrypted_answer Agora::encryptAnswer(const string &pk_json, const mpz_t &encoded_answer, const string &randomness)
{
	/**
	 * Here we not only just encrypt the answer but also provide a
	 * verifiable Proof of Knowledge (PoK) of the plaintext, using the
	 * Schnorr Protocol with Fiat-Shamir (which is a method of
	 * converting an interactive PoK into non interactive using a hash
	 * that substitutes the random oracle). We use sha256 for hashing.
	 */
	//var pk = ElGamal.PublicKey.fromJSONObject(pk_json);
	ElGamal::PublicKey pk = ElGamal::PublicKey::fromJSONObject(pk_json);
	//var plaintext = new    ElGamal.Plaintext(encoded_answer, pk, true);
	Agora::Encrypted_answer answer;
	bool b = true;
	ElGamal::Plaintext plaintext = ElGamal::Plaintext(encoded_answer, pk, b);
	mpz_t random;
	mpz_t zero;
	mpz_init(random);
	mpz_init(zero);
	//this conditional checks whether it's a (valid) number, as mpz_set_str
	//returns 0 only if randomness is a valid number in base 10
	if (0 != mpz_set_str(random,randomness.c_str(), 10))
	{
		Random::getRandomInteger(random, pk.q);
	}
	ElGamal::Ciphertext ciphertext = ElGamal::encrypt(pk, plaintext, random);
	ElGamal::Fiatshamir_dlog_challenge_generator generator;
	ElGamal::DLogProof proof = plaintext.proveKnowledge(ciphertext.alpha, random, generator);
	answer = Agora::Encrypted_answer(ciphertext.alpha, ciphertext.beta, proof.commitment, proof.response, proof.challenge);
	return answer;
}
