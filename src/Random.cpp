/*
 * Random.cpp
 *
 *  Created on: May 26, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */

#include "Random.h"
#include <fcntl.h>
#include <unistd.h>


bool Random::initiated = false;
gmp_randstate_t  Random::state;

void Random::initState() {
	int randomData = open("/dev/urandom", O_RDONLY);
	int myRandomInteger = 0;
	size_t randomDataLen = 0;
	while (randomDataLen < sizeof(myRandomInteger) ) {
		ssize_t result = read(randomData, ((char*)&myRandomInteger) + randomDataLen, sizeof(myRandomInteger) - randomDataLen);
		if (result < 0) {
			// error, unable to read /dev/random
		}
		randomDataLen += result;
	}
	close(randomData);
	gmp_randinit_mt(state); //is this random enough? does it use the random source of linux kernel?
	gmp_randseed_ui(state, myRandomInteger);
}

void Random::getRandomInteger(mpz_t &out, const mpz_t &max) {
	if(!initiated) {
		initState();
	}
	mpz_init(out);
	mpz_urandomm(out, state, max);
}



