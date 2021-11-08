// SPDX-FileCopyrightText: 2014 Brad Conte (brad AT bradconte.com)
// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Based on https://github.com/B-Con/crypto-algorithms (Public Domain from Brad
// Conte).
//
// This code defines the API for the corresponding SHA256 implementation

#ifndef SHA256_H
#define SHA256_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>

#include <string>

#include "Random.h"
using namespace std;

namespace AgoraAirgap {

namespace sha256 {

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32  // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE;  // 8-bit byte
typedef unsigned int WORD;  // 32-bit word, change to "long" for 16-bit machines

typedef struct
{
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/

string hex_sha256(const string in);
void sha256_init(SHA256_CTX * ctx);
void sha256_update(SHA256_CTX * ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX * ctx, BYTE hash[]);

}  // namespace sha256

}  // namespace AgoraAirgap

#endif  // SHA256_H
