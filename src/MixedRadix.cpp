// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <gmpxx.h>
#include <agora-airgap/MixedRadix.h>

#include <vector>
#include <stdexcept>

namespace AgoraAirgap {

namespace MixedRadix {

using std::vector;
using std::runtime_error;

mpz_class encode(
    const vector<uint32_t> & valueList,
    const vector<uint32_t> & baseList)
{
    // validate input
    if (valueList.size() != baseList.size())
    {
        throw runtime_error(
            "Invalid parameters: 'valueList' and 'baseList' must have the " \
            "same length.");
    }

    // Encode
    mpz_class encodedValue(0);
    uint32_t index = valueList.size() - 1;
    while (index >= 0)
    {
        mpz_class value = valueList[index];
        mpz_class base = baseList[index];
        encodedValue = encodedValue * base + value;
        index--;
    }
    return encodedValue;
}

vector<uint32_t> decode(
    const vector<uint32_t> & baseList,
    const mpz_class & encodedValue,
    const uint32_t * lastBase)
{
    vector<uint32_t> decodedValues;
    mpz_class accumulator = encodedValue;
    uint32_t index = 0;

    while (accumulator > 0)
    {
        mpz_class base;
        if (index >= baseList.size() && lastBase == nullptr)
        {
            throw runtime_error(
                "Error decoding: lastBase was needed but not provided");
        }

        if (index < baseList.size())
        {
            base = baseList[index];
        } else {
            base = mpz_class(*lastBase);
        }

        mpz_class remainder = accumulator % base;
        decodedValues.push_back(remainder.get_ui());
        accumulator = (accumulator - remainder) / base;
        index++;
    }

    // If we didn't run all the bases, fill the rest with zeros
    while (baseList.size() < index)
    {
        decodedValues.push_back(0);
        index++;    
    }

    return decodedValues;
}

}

}  // namespace AgoraAirgap
