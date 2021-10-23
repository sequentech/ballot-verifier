// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Loosely based on Ben Adida's jscrypto:
// https://github.com/benadida/jscrypto/blob/master/elgamal.js

#include <agora-airgap/Random.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

using namespace std;
using namespace AgoraAirgap;

#if defined(_WIN32) || defined(WIN32)
#include <wincrypt.h>
#include <windows.h>

#include <iostream>
#endif

void Random::initState()
{
#ifdef __unix__
    int randomData = open("/dev/urandom", O_RDONLY);
    unsigned long int myRandomInteger = 0;
    size_t randomDataLen = 0;
    while (randomDataLen < sizeof(myRandomInteger))
    {
        ssize_t result = read(
            randomData, ((char *) &myRandomInteger) + randomDataLen,
            sizeof(myRandomInteger) - randomDataLen);
        if (result < 0)
        {
            // error, unable to read /dev/random
        }
        randomDataLen += result;
    }
    close(randomData);
    state.seed(myRandomInteger);
#elif defined(_WIN32) || defined(WIN32)

    HCRYPTPROV hProvider = 0;
    unsigned long int myRandomInteger = 0;

    if (!CryptAcquireContextW(
            &hProvider, 0, 0, PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
    {
        exit(1);
    }

    const DWORD dwLength = 4;
    BYTE pbBuffer[dwLength] = {};

    if (!CryptGenRandom(hProvider, dwLength, pbBuffer))
    {
        CryptReleaseContext(hProvider, 0);
        return;
    }
    for (DWORD i = 0; i < dwLength; ++i)
    {
        myRandomInteger = (myRandomInteger << 8) | pbBuffer[i];
    }

    if (!CryptReleaseContext(hProvider, 0))
    {
        return;
    }
    state.seed(myRandomInteger);
    // cout << "uu: "<< mpz_get_str(NULL, 10, uu) << endl;
#endif
}

mpz_class Random::getRandomIntegerBits(const mpz_class & bits)
{
    Random * r = singleton();
    return r->state.get_z_bits(bits);
}

mpz_class Random::getRandomIntegerRange(const mpz_class & bits)
{
    Random * r = singleton();
    return r->state.get_z_range(bits);
}

Random * Random::singleton()
{
    static Random r;
    return &r;
}
