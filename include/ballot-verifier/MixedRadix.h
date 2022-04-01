// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

#ifndef MIXED_RADIX_H_
#define MIXED_RADIX_H_

#include <gmpxx.h>

#include <vector>

namespace AgoraAirgap {

namespace MixedRadix {

/**
 * Mixed number encoding.
 *
 * It will encode using multiple different bases. The number of bases and
 * the number of values need to be equal.
 *
 * @param valueList List of positive integer number values to encode
 * @param baseList  List of positive integer bases to use
 *
 * @returns the encoded number
 */
mpz_class encode(
    const std::vector<uint32_t> & valueList,
    const std::vector<uint32_t> & baseList);

/**
 * Mixed number decoding.
 *
 * It will decode using multiple different bases.
 *
 * @param baseList     List of positive integer bases to use
 * @param lastBase     Base to use if baseList is too short
 * @param encodedValue Integer value to decode
 *
 * @returns the list of positive decoded integer values
 */
std::vector<uint32_t> decode(
    const std::vector<uint32_t> & baseList,
    const mpz_class & encodedValue,
    const uint32_t * lastBase = nullptr);

}  // namespace MixedRadix

}  // namespace AgoraAirgap

#endif /* MIXED_RADIX_H_ */
