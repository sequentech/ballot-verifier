/*
 * Random.h
 *
 *  Created on: May 26, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include <gmp.h>
#include <string>

class Random
{
private:
	static gmp_randstate_t state;
	static bool initiated;
public:
	static void initState();
	static void getRandomInteger(mpz_t &out, const mpz_t &max);
};





#endif /* RANDOM_H_ */
