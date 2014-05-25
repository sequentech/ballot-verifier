/*
 * ElGamal.cpp
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */


#include "ElGamal.h"

ElGamal::SecretKey::SecretKey(const mpz_t &x, const ElGamal::PublicKey &pk) {
	mpz_init_set(this->x, x);
	this->pk = ElGamal::PublicKey(pk);
}

ElGamal::Params::Params() {
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
}

ElGamal::Params::Params(const mpz_t &p, const mpz_t &q, const mpz_t &g) {
	mpz_init_set(this->p, p);
	mpz_init_set(this->q, q);
	mpz_init_set(this->g, g);
}

ElGamal::SecretKey ElGamal::Params::generate(){

	mpz_t x, y;
	mpz_init(x);
	mpz_init(y);
	Random::getRandomInteger(x, this->q);
	mpz_powm(y, g, x, this->p); //y = (g^x) mod p

	ElGamal::PublicKey pk = ElGamal::PublicKey(this->p, this->q, this->g, y);
	ElGamal::SecretKey sk = ElGamal::SecretKey(x, pk);
	return sk;
}

ElGamal::PublicKey::PublicKey(const mpz_t &p, const mpz_t &q, const mpz_t &g, const mpz_t &y) {
	mpz_init_set(this->p, p);
	mpz_init_set(this->q, q);
	mpz_init_set(this->g, g);
	mpz_init_set(this->y, y);
 }

ElGamal::PublicKey::PublicKey(const ElGamal::PublicKey &pk) {
	mpz_init_set(p, pk.p);
	mpz_init_set(q, pk.q);
	mpz_init_set(g, pk.g);
	mpz_init_set(y, pk.y);
}

ElGamal::PublicKey::PublicKey() {

	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(y);
}
ElGamal::PublicKey ElGamal::PublicKey::fromJSONObject(const string &pk_json) {
	//TODO:
	ElGamal::PublicKey p;
	return p;
}

ElGamal::Ciphertext::Ciphertext() {
	mpz_init(alpha);
	mpz_init(beta);
}
ElGamal::Ciphertext::Ciphertext(const mpz_t &alpha, const mpz_t &beta, const PublicKey &pk) {
	mpz_init_set(this->alpha, alpha);
	mpz_init_set(this->beta, beta);
	this->pk = PublicKey(pk);
}
string ElGamal::Ciphertext::toString() {
	string s("");
	s +=  mpz_get_str(NULL, 10, alpha);
	s += ", ";
	s +=  mpz_get_str(NULL, 10, beta);
	return s;
}

ElGamal::Plaintext::Plaintext(const mpz_t &m, const PublicKey &pk, const bool &encode_m) {
	mpz_t zero;
	mpz_t one;
	mpz_init_set_str (zero, "0", 10);
	mpz_init(this->m);
	mpz_init_set_str (one, "1", 10);
	if(0 != mpz_cmp(zero, m)) {
		this->pk = pk;
		// need to encode the message given that p = 2q+1
		if(true == encode_m) {
			mpz_t foo, test;
			mpz_init(foo);
			mpz_init(test);
			mpz_add(foo, m, one); // foo = m + 1
			mpz_powm(test, foo, pk.q, pk.p); // test = (foo^q) mod p
			if(mpz_cmp(one, test)) {
				mpz_set(this->m, foo);
			}
			else {
				mpz_neg(this->m, foo);
				mpz_mod(this->m, this->m, pk.p);
			}

		}
		else {
			mpz_set(this->m, m);
		}
	}
}

void ElGamal::Plaintext::getM(mpz_t &res) const {
	mpz_init_set(res, m);
}

void ElGamal::Plaintext::getPlaintext(mpz_t &res) const {
	mpz_t y;
	mpz_t one;
	mpz_init_set_str (one, "1", 10);
	if(mpz_cmp(m, pk.q) < 0) {
		mpz_init_set(y, m);
	}
	else {
		mpz_neg(y, m);
		mpz_mod(y, y, pk.p);
	}

	mpz_sub(y, y, one);
	mpz_init_set(res, y);
}

ElGamal::PlaintextCommitment::PlaintextCommitment() {
	mpz_init(a);
	mpz_init(alpha);
}

ElGamal::PlaintextCommitment::PlaintextCommitment(const mpz_t &alpha, const mpz_t &a) {
	mpz_init_set(this->a, a);
	mpz_init_set(this->alpha, alpha);
}

string ElGamal::PlaintextCommitment::toString() const {
	string salpha( mpz_get_str(NULL, 10, alpha)), sa(mpz_get_str(NULL, 10, a));
	string out = salpha + "/" + sa;
	return out;
}

// generate a proof of knowledge of the plaintext (schnorr protocol)
// http://courses.csail.mit.edu/6.897/spring04/L19.pdf
ElGamal::DLogProof ElGamal::Plaintext::proveKnowledge(const mpz_t &alpha, const mpz_t &randomness,
		const Challenge_Generator &challenge_generator) const{

	mpz_t w;
	// generate random w
	Random::getRandomInteger(w, pk.q);

	mpz_t a;
	mpz_init(a);
	// compute first part of commitment = g^w for random w.
	mpz_powm(a, pk.g, w, pk.p);

	ElGamal::PlaintextCommitment commitment = ElGamal::PlaintextCommitment(alpha, a);

	mpz_t challenge;
	mpz_init(challenge);
	// get challenge
	challenge_generator.generator(challenge, commitment);

	// compute response = ( w + randomness * challenge ) mod pk.q
	mpz_t response;
	mpz_init(response);
	mpz_mul(response, randomness, challenge);
	mpz_add(response, response, w);
	mpz_mod(response, response, pk.q);

	ElGamal::DLogProof out = ElGamal::DLogProof(commitment, challenge, response);
	return out;

}

void ElGamal::Fiatshamir_dlog_challenge_generator::generator(mpz_t &out, const ElGamal::PlaintextCommitment &commitment) const {
	mpz_init_set_str(out, hex_sha256(commitment.toString()).c_str(), 16);
	//return new BigInt(hex_sha256(commitment.toString()), 16);
}


ElGamal::Ciphertext ElGamal::encrypt(const PublicKey &pk, const Plaintext &plaintext, const mpz_t &r) {
	mpz_t zero, m;
	mpz_init_set_str(zero, "0", 10);
	mpz_init_set_str(m, "0", 10);
	plaintext.getM(m);
	if( 0 == mpz_cmp(m, zero) ) {
		//ElGamal::Ciphertext ct;
		return ElGamal::Ciphertext(zero, zero, pk);
	}
	else {

		mpz_t random;
		mpz_init(random);
		if( 0 == mpz_cmp(r, zero) ) {
			Random::getRandomInteger(random, pk.q);
		}
		else {
			mpz_set(random, r);
		}
		mpz_t alpha, beta, m;
		mpz_init(alpha);
		mpz_init(beta);
		mpz_init(m);
		plaintext.getM(m);
		mpz_powm(alpha, pk.g, random, pk.p); // alpha = (g^random) mod p
		mpz_powm(beta, pk.y, random, pk.p);
		mpz_mul(beta, beta, m);
		mpz_mod(beta, beta, pk.p); // beta = ( (y^random) M  ) mod p
		return ElGamal::Ciphertext(alpha, beta, pk);
	}
}

ElGamal::DLogProof::DLogProof() {
	mpz_init(challenge);
	mpz_init(response);
}

ElGamal::DLogProof::DLogProof(const ElGamal::PlaintextCommitment &commitment , const mpz_t &challenge, const mpz_t &response) {
	this->commitment = commitment;
	mpz_init_set(this->challenge, challenge);
	mpz_init_set(this->response, response);
}
