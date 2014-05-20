/*
 * sha2.h
 *
 *  Created on: May 20, 2014
 *      Author: felix
 */

#ifndef SHA2_H_
#define SHA2_H_

#include <string>
#include <gmp.h>
using namespace std;

string hex_sha256(const mpz_t alpha, const mpz_t a);




#endif /* SHA2_H_ */
