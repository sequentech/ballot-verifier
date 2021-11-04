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
    vector<uint32_t> getBases();
};

}  // namespace AgoraAirgap

#endif /* NVOTES_CODEC_H_ */