/*
 * Agora.cpp
 *
 *  Created on: May 18, 2014
 *      Author: F�lix Robles felrobelv at gmail dot com
 * Loosely based on Agora Voting:
 * https://github.com/agoraciudadana/agora-ciudadana/blob/security/agora_site/static/js/agora/views/voting_booth.js
 */


#include "Agora.h"

Agora::Encrypted_answer::Encrypted_answer()
: ElGamal::Ciphertext()
{
	mpz_init(challenge);
	mpz_init(response);
}

Agora::Encrypted_answer::Encrypted_answer(const mpz_t &alpha, const mpz_t &beta, const PublicKey &pk, const ElGamal::PlaintextCommitment &commitment,
				const mpz_t &challenge,const mpz_t &response):
        ElGamal::Ciphertext(alpha, beta, pk)
{
	this->commitment = commitment;
	mpz_init_set(this->challenge, challenge);
	mpz_init_set(this->response, response);
}
Agora::Encrypted_answer Agora::encryptAnswer(const ElGamal::PublicKey &pk, const mpz_t &encoded_answer, const mpz_t &randomness) {
	/**
	 * Here we not only just encrypt the answer but also provide a
	 * verifiable Proof of Knowledge (PoK) of the plaintext, using the
	 * Schnorr Protocol with Fiat-Shamir (which is a method of
	 * converting an interactive PoK into non interactive using a hash
	 * that substitutes the random oracle). We use sha256 for hashing.
	 */
	Agora::Encrypted_answer answer;
	bool b = true;
	ElGamal::Plaintext plaintext = ElGamal::Plaintext(encoded_answer, pk, b);
	mpz_t zero, random;
	mpz_init(zero);
	mpz_init(random);
	//returns 0 only if randomness == 0
	if (0 == mpz_cmp(randomness,zero)) 	{
		Random::getRandomInteger(random, pk.q);
	} else if (mpz_cmp(randomness,pk.q) > 0 ) { //randomness is too big
		mpz_mod(random, randomness, pk.q);
	}
	else {
		mpz_set(random, randomness);
	}
	ElGamal::Ciphertext ciphertext = ElGamal::encrypt(pk, plaintext, random);
	ElGamal::Fiatshamir_dlog_challenge_generator generator;
	ElGamal::DLogProof proof = plaintext.proveKnowledge(ciphertext.alpha, random, generator);
	answer = Agora::Encrypted_answer(ciphertext.alpha, ciphertext.beta, proof.commitment, proof.response, proof.challenge);
	return answer;
}


ElGamal::Plaintext Agora::decryptAnswer(const SecretKey &sk, const Encrypted_answer & encryptedanswer) {
  ElGamal::Plaintext cleartext = ElGamal::decrypt(sk, encryptedanswer);
  return cleartext;
}
