/*
 * Random.cpp
 *
 *  Created on: May 26, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */

#include "Random.h"

void Random::getRandomInteger(mpz_t out, const mpz_t max)
{
	gmp_randstate_t state;
	gmp_randinit_mt(state); //is this random enough? does it use the random source of linux kernel?
	mpz_init(out);
	mpz_urandomm(out, state, max);
}



