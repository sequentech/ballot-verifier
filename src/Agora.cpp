/*
 * Agora.cpp
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Agora Voting:
 * https://github.com/agoraciudadana/agora-ciudadana/blob/security/agora_site/static/js/agora/views/voting_booth.js
 */


#include "Agora.h"


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
	mpz_init_set_str(random, "0", 10);
	mpz_init_set_str(zero, "0", 10);
	//this conditional checks whether it's a (valid) number, as mpz_set_str
	//returns 0 only if randomness is a valid number in base 10
	if (0 != mpz_set_str(random,randomness.c_str(), 10))
	{
		gmp_randstate_t state;
		gmp_randinit_mt(state); //is this random enough? does it use the random source of linux kernel?
		mpz_urandomm(random, state, pk.q);
	}
	ElGamal::Ciphertext ct = ElGamal::encrypt(pk, plaintext, random);
	return a;
}
