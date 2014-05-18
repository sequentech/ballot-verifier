/*
 * ElGamal.cpp
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */


#include "ElGamal.h"

ElGamal::Plaintext::Plaintext(mpz_t m, PublicKey pk, bool encode_m)
{
	mpz_t zero;
	mpz_t one;
	mpz_init_set_str (zero, "0", 10);
	mpz_init_set_str (one, "1", 10);
	if(0 != mpz_cmp(zero, m))
	{
		this->pk = pk;
		// need to encode the message given that p = 2q+1
		if(true == encode_m)
		{
			mpz_t foo, test;
			mpz_add(foo, m, one); // foo = m + 1
			mpz_powm(test, foo, pk.q, pk.p); // test = (foo^q) mod p
			if(mpz_cmp(one, test))
			{
				mpz_init_set(this->m, foo);
			}
			else
			{
				mpz_neg(this->m, foo);
				mpz_mod(this->m, this->m, pk.p);
			}

		}
		else
		{
			mpz_init_set(this->m, m);
		}
	}
}
void ElGamal::Plaintext::getM(mpz_t res) const
{
	mpz_init_set(res, m);
}
void ElGamal::Plaintext::getPlaintext(mpz_t res) const
{
	mpz_t y;
	mpz_t one;
	mpz_init_set_str (one, "1", 10);
	if(mpz_cmp(m, pk.q) < 0)
	{
		mpz_init_set(y, m);
	}
	else
	{
		mpz_neg(y, m);
		mpz_mod(y, y, pk.p);
	}

	mpz_sub(y, y, one);
	mpz_init_set(res, y);
}

/*if (plaintext.getM().equals(BigInt.ZERO))
	    throw "Can't encrypt 0 with El Gamal"


	  if (!r)
	    r = Random.getRandomInteger(pk.q);

	  var alpha = pk.g.modPow(r, pk.p);
	  var beta = (pk.y.modPow(r, pk.p)).multiply(plaintext.m).mod(pk.p);

	  return new ElGamal.Ciphertext(alpha, beta, pk);
	  */

ElGamal::Ciphertext ElGamal::encrypt(const PublicKey pk, const Plaintext plaintext, const mpz_t r)
{
	mpz_t zero, m;
	mpz_init_set_str(zero, "0", 10);
	mpz_init_set_str(m, "0", 10);
	plaintext.getM(m);
	if( 0 == mpz_cmp(m, zero) )
	{
		//ElGamal::Ciphertext ct;
		return ElGamal::Ciphertext(zero, zero, pk);
	}
	else
	{

		mpz_t random;
		if( 0 == mpz_cmp(r, zero) )
		{
			gmp_randstate_t state;
			gmp_randinit_mt(state); //is this random enough? does it use the random source of linux kernel?
			mpz_init(random);
			mpz_urandomm(random, state, pk.q);
		}
		else
		{
			mpz_init_set(random, r);
		}
		mpz_t alpha, beta, m;
		mpz_init(alpha);
		mpz_init(beta);
		mpz_init(m);
		plaintext.getM(m);
		mpz_powm(alpha, pk.g, random, pk.p);
		mpz_powm(beta, pk.y, random, pk.p);
		mpz_mul(beta, beta, m);
		mpz_mod(beta, beta, pk.p);
		return ElGamal::Ciphertext(alpha, beta, pk);
	}
}












