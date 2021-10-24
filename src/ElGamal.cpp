// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Based on George Danezis/Ben Adida/Eduardo Robles
// https://github.com/agoravoting/agora-api

#include <agora-airgap/ElGamal.h>
#include <agora-airgap/Random.h>

#include <iostream>
#include <sstream>

namespace AgoraAirgap {

ElGamal::PublicKey::PublicKey(
    const mpz_class & ap,
    const mpz_class & aq,
    const mpz_class & ag,
    const mpz_class & ay)
    : p(ap), q(aq), g(ag), y(ay)
{}

ElGamal::PublicKey ElGamal::PublicKey::fromJSONObject(const Value & pk)
{
    mpz_class p, q, g, y;
    p = pk["p"].GetString();
    q = pk["q"].GetString();
    g = pk["g"].GetString();
    y = pk["y"].GetString();

    return PublicKey(p, q, g, y);
}

ElGamal::Ciphertext::Ciphertext(
    const mpz_class & aalpha,
    const mpz_class & abeta,
    const PublicKey & apk)
    : alpha(aalpha), beta(abeta), pk(apk)
{}

ElGamal::Plaintext::Plaintext(
    const mpz_class & am,
    const ElGamal::PublicKey & apk,
    const bool & encode_m)
    : pk(apk)
{
    if (encode_m)
    {
        // need to encode the message given that p = 2q+1
        mpz_class y = am + 1;

        mpz_class test;
        mpz_powm(
            test.get_mpz_t(),
            y.get_mpz_t(),
            pk.q.get_mpz_t(),
            pk.p.get_mpz_t());  // test = (y^q) mod p

        if (test == 1)
        {
            m = y;
        } else
        {
            mpz_class neg;
            neg = -y;
            mpz_mod(neg.get_mpz_t(), neg.get_mpz_t(), pk.p.get_mpz_t());
            m = neg;
        }
    } else
    {
        m = am;
    }
}

ElGamal::Ciphertext ElGamal::encrypt(
    const ElGamal::PublicKey & pk,
    const ElGamal::Plaintext & plaintext,
    const mpz_class & rr)
{
    mpz_class r = rr;
    if (0 == plaintext.m)
    {
        mpz_class a, b;
        a = 0;
        b = 0;
        Ciphertext ret(a, b, pk);
        return ret;
    }
    if (0 == r)
    {
        r = Random::getRandomIntegerRange(pk.q);
    }
    mpz_class alpha, beta;
    mpz_powm(
        alpha.get_mpz_t(),
        pk.g.get_mpz_t(),
        r.get_mpz_t(),
        pk.p.get_mpz_t());  // alpha = (g^r) mod p
    mpz_powm(
        beta.get_mpz_t(),
        pk.y.get_mpz_t(),
        r.get_mpz_t(),
        pk.p.get_mpz_t());  // beta = ((y^r mod p) * m ) mod p
    mpz_mul(beta.get_mpz_t(), beta.get_mpz_t(), plaintext.m.get_mpz_t());
    mpz_mod(beta.get_mpz_t(), beta.get_mpz_t(), pk.p.get_mpz_t());

    Ciphertext ret(alpha, beta, pk);
    return ret;
}

std::string ElGamal::PlaintextCommitment::toString() const
{
    std::stringstream ss;
    ss << mpz_get_str(NULL, 10, alpha.get_mpz_t()) << "/"
       << mpz_get_str(NULL, 10, a.get_mpz_t());
    return ss.str();
}

std::string ElGamal::PlaintextCommitment::toJSON() const
{
    std::stringstream ss;

    ss << "{\"alpha\":\"" << alpha.get_str(10) << "\","
       << "\"a\":\"" << a.get_str(10) << "\"}";

    return ss.str();
}

mpz_class ElGamal::Fiatshamir_dlog_challenge_generator::generator(
    const ElGamal::PlaintextCommitment & commitment) const
{
    mpz_class out;
    mpz_init_set_str(
        out.get_mpz_t(), sha256::hex_sha256(commitment.toString()).c_str(), 16);
    return out;
}

ElGamal::DLogProof ElGamal::Plaintext::proveKnowledge(
    const mpz_class & alpha,
    const mpz_class & randomness,
    const ChallengeGenerator & challenge_generator)
{
    mpz_class w, a, challenge, response;

    w = Random::getRandomIntegerRange(pk.q);
    mpz_powm(a.get_mpz_t(), pk.g.get_mpz_t(), w.get_mpz_t(), pk.p.get_mpz_t());

    PlaintextCommitment commitment(alpha, a);

    challenge = challenge_generator.generator(commitment);
    // var response = w.add(randomness.multiply(challenge)).mod(this.pk.q);
    mpz_mul(
        response.get_mpz_t(), randomness.get_mpz_t(), challenge.get_mpz_t());
    mpz_add(response.get_mpz_t(), response.get_mpz_t(), w.get_mpz_t());
    mpz_mod(response.get_mpz_t(), response.get_mpz_t(), pk.q.get_mpz_t());

    DLogProof d(commitment, challenge, response);
    return d;
}

bool ElGamal::Ciphertext::verifyPlaintextProof(
    const ElGamal::DLogProof & proof,
    const ElGamal::ChallengeGenerator & challenge_generator) const
{
    mpz_class challenge2, right, left;
    int res;
    challenge2 = challenge_generator.generator(proof.commitment);
    if (0 != mpz_cmp(challenge2.get_mpz_t(), proof.challenge.get_mpz_t()))
    {
        cout << "!!! Error verifying plaintext proof" << endl;
        return false;
    }
    mpz_powm(
        left.get_mpz_t(),
        pk.g.get_mpz_t(),
        proof.response.get_mpz_t(),
        pk.p.get_mpz_t());
    mpz_powm(
        right.get_mpz_t(),
        alpha.get_mpz_t(),
        proof.challenge.get_mpz_t(),
        pk.p.get_mpz_t());
    mpz_mul(
        right.get_mpz_t(), right.get_mpz_t(), proof.commitment.a.get_mpz_t());
    mpz_mod(right.get_mpz_t(), right.get_mpz_t(), pk.p.get_mpz_t());

    // right == left ?
    // g^response == commitment * (g^t) ^ challenge == commitment * (alpha) ^
    // challenge ?
    res = mpz_cmp(left.get_mpz_t(), right.get_mpz_t());

    return res == 0;
}

ElGamal::DLogProof::DLogProof(
    PlaintextCommitment commitment_,
    mpz_class challenge_,
    mpz_class response_)
    : commitment(commitment_), challenge(challenge_), response(response_)
{}

}  // namespace AgoraAirgap
