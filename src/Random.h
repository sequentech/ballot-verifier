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

#include <gmpxx.h>
#include <string>

class Random
{
protected:
	Random()
	: state(gmp_randinit_default)
	{ initState(); }
	gmp_randclass state;
	void initState();
	static Random* singleton();
public:
	static mpz_class getRandomIntegerBits(const mpz_class &bits);
	static mpz_class getRandomIntegerRange(const mpz_class &max);
};





#endif /* RANDOM_H_ */
