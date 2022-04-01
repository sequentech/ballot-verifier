// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Loosely based on Ben Adida's jscrypto:
// https://github.com/benadida/jscrypto/blob/master/elgamal.js

#ifndef RANDOM_H_
#define RANDOM_H_

#include <gmpxx.h>

#include <string>

namespace AgoraAirgap {
class Random
{
    protected:
    Random() : state(gmp_randinit_default) { initState(); }

    gmp_randclass state;

    void initState();
    static Random * singleton();

    public:
    static mpz_class getRandomIntegerBits(const mpz_class & bits);
    static mpz_class getRandomIntegerRange(const mpz_class & max);
};

}  // namespace AgoraAirgap

#endif /* RANDOM_H_ */
