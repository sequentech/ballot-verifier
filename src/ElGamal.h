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

using namespace std;

class ElGamal
{
public:
	class PublicKey
	{
	public:
		mpz_t p, q, g, y;
		PublicKey(const mpz_t pp, const mpz_t qq, const mpz_t gg, const mpz_t yy)
		{
			mpz_init_set(p, pp);
			mpz_init_set(q, qq);
			mpz_init_set(g, gg);
			mpz_init_set(y, yy);
		 }
		PublicKey(const ElGamal::PublicKey &pk)
		{

			mpz_init_set(p, pk.p);
			mpz_init_set(q, pk.q);
			mpz_init_set(g, pk.g);
			mpz_init_set(y, pk.y);
		}
		PublicKey()
		{

			mpz_init(p);
			mpz_init(q);
			mpz_init(g);
			mpz_init(y);
		}
		static PublicKey fromJSONObject(string pk_json)
		{
			//TODO:
			PublicKey p;
			return p;
		}
	};

	class Plaintext
	{
	public:
		PublicKey pk;
		Plaintext(mpz_t m, PublicKey pk, bool encode_m);
		void getPlaintext(mpz_t res) const;
		void getM(mpz_t res) const;
	private:
		mpz_t m;

	};


	class Ciphertext{
	public:
			PublicKey pk;
			mpz_t alpha, beta;
			Ciphertext()
			{
				mpz_init(alpha);
				mpz_init(beta);
			}
			Ciphertext(mpz_t alpha, mpz_t beta, PublicKey pk)
			{
				mpz_init_set(this->alpha, alpha);
				mpz_init_set(this->beta, beta);
				this->pk = PublicKey(pk);
			}
			string toString()
			{
				string s("");
				s +=  mpz_get_str(NULL, 10, alpha);
				s += ", ";
				s +=  mpz_get_str(NULL, 10, beta);
				return s;
			}

	};

	static ElGamal::Ciphertext encrypt(const PublicKey pk, const Plaintext plaintext, const mpz_t r);
};



#endif /* ELGAMAL_H_ */

