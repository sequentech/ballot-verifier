// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#ifndef NVOTES_CODEC_H_
#define NVOTES_CODEC_H_

#include <gmpxx.h>

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

    /**
     * Converts a raw ballot into an encoded number ready to be encrypted.
     * A raw ballot is a list of positive integer numbers representing
     * the ballot, and can be obtained calling to `self.encode_raw_ballot()`.
     *
     * Encoding is done using mixed radix encoding. The bases are
     * automatically calculated when instancing this object. The bases
     * used are either the number of points assigned to each answer or the
     * position in which that answer was selected for preferential
     * elections. Please refer to mixed radix documentation to understand
     * how it works or read https://en.wikipedia.org/wiki/Mixed_radix
     *
     * # Basics
     *
     * If in a `plurality-at-large` there are three candidates `A`, `B`,
     * and `C` with answer ids `0`, `1` and `2`, and the voter wants to
     * vote to candidates `A` and `C`, then his ballot choices (obtained
     * using encode_raw_ballot) will be  `v = [1, 0, 1]` and the encoded
     * choices will be encoded this way:
     *
     * ```
     * encoded_choices = v[0] + v[1]*b[0] + v[2]*b[0]*b[1]
     * encoded_choices = v[0] + b[0]*(v[1] + b[1]*v[2])
     * encoded_choices = 1 + 2*(0 + 2 * 1) = 1 + 4*1 = 5
     * ```
     *
     * And the bases are `b = [2, 2, 2]`. The reason the bases are 2 here
     * is because plurality-at-large is a non-preferential voting system
     * and each base is representing if the voter chose (then we use
     * `v[x] = 1`) or not (then we use `v[x] = 0`), and the base is in
     * this case max(v[x])+1`.
     *
     * # Preferential systems
     *
     * In a preferential system, the voter can choose a specific ordering.
     * If we reuse the previous example, the voter might have chosen for
     * the first choice in his ballot candidate `A`, and for his second
     * choice candidate `B`. Not choosing a candidate would be encoded as
     * value `0`, so choosing it as first position would be value `1` and
     * so on. If the voter can choose up to 3 candidates, then the base
     * would be `maxChoices+1 = 3+1 = 4`, and thus bases will be
     * `b = [4, 4, 4]` and choices would be `v = [1, 0, 2]` and the
     * encoded choices would be calculated as:
     *
     * ```
     * encoded_choices = v[0] + v[1]*b[1] + v[2]*b[1]*b[2]
     * encoded_choices = v[0] + b[0]*(v[1] + b[1]*v[2])
     * encoded_choices = 1 + 4*(0 + 4*2) = 1 + 16*2 = 33
     * ```
     *
     * # Invalid Ballot Flag
     *
     * What was outlined before is the basics, but actually it does not
     * work exactly like that. The first value (`v[0]`) in the raw ballot
     * does not really represent the vote for the first candidate answer,
     * but it's always a flag saying if the ballot was marked as invalid
     * or not by the voter. Note that this is not the only way to create
     * an invalid ballot. For example the voter could vote to more options
     * than allowed, and that would also be an invalid ballot.
     *
     * We asumes the invalid ballot flag is represented in the question
     * as a answer inside `question.answers` and it is flagged  by having
     * an element in `answer.urls` as
     * `{"title":'invalidVoteFlag', "url":'true'}`.
     *
     * Using the last example of a preferential vote, the bases would not
     * be `b = [4, 4, 4]` but `b = [2, 4, 4, 4]` (the first base encodes
     * always the invalid flag, whose max value is 1 so the base is always
     * 2).
     *
     * The choices would not be `v = [1, 0, 2]` but (if the vote was
     * not marked as invalid) `v = [0, 1, 0, 2]` and thus the encoded
     * choices would be calculated as:
     *
     * ```
     * encoded_choices = v[0] + b[0]*(v[1] + b[1]*(v[2] + b[2]*v[3])
     * encoded_choices = 0 + 2*(1 + 4*(0 + 4*2)) = 2*1 + 2*4*4*2
     * encoded_choices = 2*1 + 32*2 = 66
     * ```
     *
     * # Cumulative voting system
     *
     * In a cumulative voting system, the voter would have a total number
     * of integer points to assign to candidates, and the voter can assign
     * them to the available candidates with a maximum number of options
     * that can be assigned to each candidate.
     *
     * For example, the voter might be able to assign up to 2 points to
     * each candidate and assign a total of 3 points. In practice, the
     * encoding is done in a very similar format as with preferential
     * voting system. For each candidate, the value we assign is a number
     * that represents the points assigned to the candidate, and the base
     * used is the maximum number of assignable points plus one.
     *
     * Retaking the previous example used for plurality-at-large and used
     * for a preferential voting system, if the voter can assign a
     * maximum of 4 points, and he wants to assign 2 points to candidate
     * `A` and 2 points to candidate `C` and he didn't mark his ballot
     * as invalid, then his choices would be `v = [0, 2, 0, 1]`, the bases
     * would be `b = [2, 5, 5, 5]` and the encoded choices would be
     * calculated as:
     *
     * ```
     * encoded_choices = v[0] + b[0]*(v[1] + b[1]*(v[2] + b[2]*v[3])
     * encoded_choices = 0 + 2*(2 + 5*(0 + 5*1)) = 2*2 + 2*5*5*1
     * encoded_choices = 2*2 + 50*1 = 54
     * ```
     *
     * # Write-ins
     *
     * This encoder supports write-ins. The idea of write-ins is that the
     * voter can choose candidates that are not in the preconfigured list
     * of candidates. The maximum number of write-ins allowed is
     * calculated automatically by suppossing the voter tries to
     * distribute his vote entirely just for write-in candidates, which
     * is usually `question.max`.
     *
     * The vote for each write-in is encoded using the same procedure as
     * for normal candidates, in order and as if the write-ins were in
     * the list of candidates. It asumes all write-ins (even if not
     * selected) are in the list of candidates and they are flagged as
     * such simply by an element in `answer.urls` as
     * `{"title":'isWriteIn', "url":'true'}`.
     *
     * For example in a plurality-at-large question example with three
     * candidates `A`, `B` and `C` where the voter can choose up to 2
     * candidates, if the voter wants to cast a valid ballot to his 2
     * write-ins, then the bases, the choices and the encoded choices
     * would be:
     *
     * ```
     * // bases
     * b = [2, 2, 2, 2, 2, 2]
     * // choices
     * v = [0, 0, 0, 0, 1, 1]
     * encoded_choices = 1*2^4 + 1*2^5 = 48
     * ```
     *
     * # Write-in names
     *
     * Of course that's not where a vote with write-ins ends. If the voter
     * voted to the write-ins, we would also have to encode the free text
     * string of the name of the write-ins. This is done by converting the
     * text from UTF-8 to numeric bytes, and encoding each byte using
     * 2^8 = 256 as a base. The separation between the different write-in
     * names is done using an empty byte (so `v[x] = 0`).
     *
     * So if in our case the name of the voter's two write-ins is `D` and
     * `E`, and knowing that character D is encoded as number `68` and E
     * is `69`, then the bases, the choices and the encoded choices
     * would be:
     *
     * ```
     * // bases
     * b = [2, 2, 2, 2, 2, 2, 256, 256, 256, 256]
     * // choices
     * v = [0, 0, 0, 0, 1, 1, 68,  0,   69,  0]
     * encoded_choices = 1*2^4 + 1*2^5 + 68*2^6 + 69*2^8 = 22064
     * ```
     */
    mpz_class encodeToInt(const RawBallot & rawBallot) const;
};

}  // namespace AgoraAirgap

#endif /* NVOTES_CODEC_H_ */