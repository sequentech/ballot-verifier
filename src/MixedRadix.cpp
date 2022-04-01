// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include <ballot-verifier/MixedRadix.h>
#include <gmpxx.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace AgoraAirgap {

namespace MixedRadix {

using std::runtime_error;
using std::stringstream;
using std::vector;

mpz_class encode(
    const vector<uint32_t> & valueList,
    const vector<uint32_t> & baseList)
{
    // validate input
    if (valueList.size() != baseList.size())
    {
        stringstream errorMessage;
        errorMessage << "Invalid parameters: 'valueList' (size = "
                     << valueList.size()
                     << ") and 'baseList' (size = " << baseList.size()
                     << ") must have the same length.";
        throw runtime_error(errorMessage.str());
    }

    // Encode
    mpz_class encodedValue(0);
    int32_t index = static_cast<int32_t>(valueList.size() - 1);
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
            stringstream errorMessage;
            errorMessage << "Error decoding: lastBase was needed but not "
                         << "provided, with index = " << index
                         << " and decodedValues[index] = " << decodedValues[1];
            throw runtime_error(errorMessage.str());
        }

        if (index < baseList.size())
        {
            base = baseList[index];
        } else
        {
            base = mpz_class(*lastBase);
        }

        mpz_class remainder = accumulator % base;
        decodedValues.push_back(remainder.get_ui());
        accumulator = (accumulator - remainder) / base;
        index++;
    }

    // If we didn't run all the bases, fill the rest with zeros
    while (index < baseList.size())
    {
        decodedValues.push_back(0);
        index++;
    }

    return decodedValues;
}

}  // namespace MixedRadix

}  // namespace AgoraAirgap
