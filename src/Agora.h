/*
 * Agora.h
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Loosely based on Agora Voting:
 * https://github.com/agoraciudadana/agora-ciudadana/blob/security/agora_site/static/js/agora/views/voting_booth.js
 */


#ifndef AGORA_H_
#define AGORA_H_


#include "ElGamal.h"


class Agora{
public:
	typedef struct {
		int a;
	}encrypted_answer;
	static encrypted_answer encryptAnswer(string pk_json, mpz_t encoded_answer, string randomness);
};



#endif /* AGORA_H_ */
