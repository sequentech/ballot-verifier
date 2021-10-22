/*
 * ElGamal.h
 *
 *  Created on: May 18, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Based on George Danezis/Ben Adida/Eduardo Robles
 * https://github.com/agoravoting/agora-api
 */
#ifndef ELGAMAL_H
#define ELGAMAL_H

#include "common.h"
#include <gmpxx.h>
#include "sha256.h"

using namespace rapidjson;

namespace AgoraAirgap {

class ElGamal {
public:
	class ChallengeGenerator;
	class DLogProof;
	
	class PublicKey {
	public:
		PublicKey(
			const mpz_class & ap, 
			const mpz_class & aq, 
			const mpz_class & ag, 
			const mpz_class & ay
		);
		
		mpz_class p, q, g, y;
		
		static PublicKey fromJSONObject(const Value & pk);
	};
	
	class Ciphertext {
	public:
		mpz_class alpha, beta;
		PublicKey pk;

		Ciphertext(
			const mpz_class & aalpha,
			const mpz_class & abeta,
			const PublicKey & apk
		);
		bool verifyPlaintextProof(
			const DLogProof & proof,
			const ChallengeGenerator & challenge_generator
		) const;
	};


	class PlaintextCommitment {
	public:
		mpz_class alpha;
		mpz_class a;
		PlaintextCommitment(
			const mpz_class & alpha_,
			const mpz_class & a_
		)
			: alpha(alpha_), a(a_)
		{}

		std::string toString() const;
		std::string toJSON() const;
	};
	

	class DLogProof {
	public:
		PlaintextCommitment commitment;
		mpz_class challenge;
		mpz_class response;
		
		DLogProof(
			PlaintextCommitment commitment_, 
			mpz_class challenge_,
			mpz_class response_
		);
	};
	
	
	class Plaintext {
	public:
		PublicKey pk;
		mpz_class m;
		Plaintext(
			const mpz_class & am, 
			const PublicKey & apk, 
			const bool & encode_m
		);
		
		DLogProof proveKnowledge(
			const mpz_class & alpha,
			const mpz_class & randomness,
			const ChallengeGenerator & challenge_generator
		);
	};
	
	class ChallengeGenerator {
	public:
		virtual mpz_class generator(
			const PlaintextCommitment & commitment
		) const = 0;
	};
	
	class Fiatshamir_dlog_challenge_generator
		: public ChallengeGenerator
	{
	public:
		Fiatshamir_dlog_challenge_generator() {}
		
		mpz_class generator(const PlaintextCommitment & commitment) const;
	};
	
	static Ciphertext encrypt(
		const PublicKey & pk,
		const Plaintext & plaintext,
		const mpz_class & r
	);
};

} // namespace AgoraAirgap

#endif //ELGAMAL_H