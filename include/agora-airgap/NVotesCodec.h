// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#ifndef NVOTES_CODEC_H_
#define NVOTES_CODEC_H_

#include <vector>

#include "common.h"

namespace AgoraAirgap {

using rapidjson::Document;
using std::vector;

/**
 * Encodes a raw ballot.
 */
class RawBallot
{
    public:
    vector<uint32_t> bases;
    vector<uint32_t> choices;
};

/**
 * Encodes/Decodes the answer to a question given the question type. The encoder
 * function always receives answer as a list of answer ids.
 */
class NVotesCodec
{
    private:
    Document question;

    public:
    NVotesCodec(const Document & question);

    /**
     * @returns the bases related to this question.
     */
    vector<uint32_t> getBases() const;

    /**
     * @returns the ballot choices and the bases to be used for encoding as an
     * object, for example something like:
     *
     * ```
     * dict(
     *   choices=[0, 0, 0, 0, 1, 1, 68,  0,   69,  0],
     *   bases=[  2, 2, 2, 2, 2, 2, 256, 256, 256, 256]
     * )
     * ```
     *
     * Please read the description of the NVotesCodec::encode function for
     * details on the output format of the raw ballot.
     */
    RawBallot encodeRawBallot() const;
};

}  // namespace AgoraAirgap

#endif /* NVOTES_CODEC_H_ */