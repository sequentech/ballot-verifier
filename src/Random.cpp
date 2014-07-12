/*
 * Random.cpp
 *
 *  Created on: May 26, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Ben Adida's jscrypto:
 * https://github.com/benadida/jscrypto/blob/master/elgamal.js
 */

#include "Random.h"
#include <gmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#if defined(_WIN32) || defined(WIN32) 
	#include <iostream>
	#include <windows.h>
	#include <Wincrypt.h>
#endif

bool Random::initiated = false;
gmp_randstate_t  Random::state;

void Random::initState() {
#ifdef __unix__         
	int randomData = open("/dev/urandom", O_RDONLY);
	unsigned long int myRandomInteger = 0;
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
#elif defined(_WIN32) || defined(WIN32) 

	HCRYPTPROV hProvider = 0;
	unsigned long int myRandomInteger = 0;
 
	if (!CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
		exit(1);
	}
	 
	const DWORD dwLength = 4;
	BYTE pbBuffer[dwLength] = {};
	 
	if (!CryptGenRandom(hProvider, dwLength, pbBuffer))
	{
		CryptReleaseContext(hProvider, 0);
		return;
	}
	for (DWORD i = 0; i < dwLength; ++i) {
		myRandomInteger = (myRandomInteger << 8) | pbBuffer[i];
	}
	 
	if (!CryptReleaseContext(hProvider, 0)){
		return;
	}
	mpz_t uu;
	mpz_init_set_ui(uu, myRandomInteger);
	gmp_randinit_default(state);
	gmp_randseed(state, uu);
	cout << "uu: "<< mpz_get_str(NULL, 10, uu) << endl;
#endif	
}

void Random::getRandomInteger(mpz_t &out, const mpz_t &max) {
	if(!initiated) {
		initState();
	}
	mpz_urandomm(out, state, max);
}



