/*
 * Agora.h
 *
 *  Created on: May 18, 2014
 *      Author: F�lix Robles felrobelv at gmail dot com
 * Loosely based on Agora Voting:
 * https://github.com/agoraciudadana/agora-ciudadana/blob/security/agora_site/static/js/agora/views/voting_booth.js
 */


#ifndef AGORA_H_
#define AGORA_H_


#include "ElGamal.h"


class Agora
{
public:
	class Encrypted_answer: public ElGamal::Ciphertext  
	{
	public:
		ElGamal::PlaintextCommitment commitment;
		mpz_t challenge,response;
		Encrypted_answer();
		Encrypted_answer(const mpz_t &alpha, const mpz_t &beta, const ElGamal::PublicKey &pk,
      const ElGamal::PlaintextCommitment &commitment,	const mpz_t &challenge, const mpz_t &response);
	};
	static Encrypted_answer encryptAnswer(const ElGamal::PublicKey &pk, const mpz_t &encoded_answer, const mpz_t &random);
  static ElGamal::Plaintext decryptAnswer(const ElGamal::SecretKey &sk, const Encrypted_answer & ea);
};



#endif /* AGORA_H_ */
