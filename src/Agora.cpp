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


/*
 * Agora.encryptAnswer = function(pk_json, encoded_answer, randomness) {
        **
         * Here we not only just encrypt the answer but also provide a
         * verifiable Proof of Knowledge (PoK) of the plaintext, using the
         * Schnorr Protocol with Fiat-Shamir (which is a method of
         * converting an interactive PoK into non interactive using a hash
         * that substitutes the random oracle). We use sha256 for hashing.
         *
        var pk = ElGamal.PublicKey.fromJSONObject(pk_json);
        var plaintext = new
        ElGamal.Plaintext(encoded_answer, pk, true);
        if (!randomness) {
          randomness = Random.getRandomInteger(pk.q);
        } else {
          randomness = BigInt.fromJSONObject(randomness);
        }
        var ctext = ElGamal.encrypt(pk, plaintext, randomness);
        var proof = plaintext.proveKnowledge(ctext.alpha, randomness, ElGamal.fiatshamir_dlog_challenge_generator);
        var ciphertext =  ctext.toJSONObject();
        var json_proof = proof.toJSONObject();
        var enc_answer = {
            alpha: ciphertext.alpha,
            beta: ciphertext.beta,
            commitment: json_proof.commitment,
            response: json_proof.response,
            challenge: json_proof.challenge
        };
        return enc_answer;
 */

Agora::encrypted_answer Agora::encryptAnswer(string pk_json, mpz_t encoded_answer, string randomness)
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
	Agora::encrypted_answer a;
	ElGamal::Plaintext plaintext = ElGamal::Plaintext(encoded_answer, pk, true);
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
	ElGamal::Ciphertext ctext = ElGamal::encrypt(pk, plaintext, random);
	ElGamal::Fiatshamir_dlog_challenge_generator generator;
	ElGamal::DLogProof proof = plaintext.proveKnowledge(ctext.alpha, random, generator);
	return a;
}
