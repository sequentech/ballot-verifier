#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <cryptopp/elgamal.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <memory.h>
#include <string.h>
#include <gtest/gtest.h>
#include "Agora.h"

using namespace std;


TEST(ElGamalIntegrationTest, SmallEncryptDecryptIntegration) {
	
	mpz_t p, q, g, y, x, m, rand;
	string alpha, beta;
	string sm;

	mpz_init_set_str(p, "1019", 10);
	mpz_init_set_str(q, "359", 10); //q = (p-1)/2 = 359
	mpz_init_set_str(g, "3", 10);
	mpz_init_set_str(y, "770", 10); //y = (g^x) mod p
	mpz_init_set_str(x, "66", 10);
	mpz_init_set_str(m, "133", 10);
		//ElGamal::Params params = ElGamal::Params(p, q, g);
		//ElGamal::SecretKey sk = params.generate();
		//ElGamal::PublicKey pk = sk.pk;
	ElGamal::PublicKey pk(p, q, g, y);
	ElGamal::SecretKey sk(x, pk);
	mpz_init(rand);
	Random::getRandomInteger(rand, pk.q);
	//alpha = (g^random) mod p  = (3^4) mod 1019 = 81    if random = rand = 4
	//U = (h^random) mod p = (770^4) mod 1019 = 508
	//M = m+1 = 134
	//beta = ( U M  ) mod p = ( 508 * 134) mod 1019 = 818
	Agora::Encrypted_answer coded_m = Agora::encryptAnswer(pk, m, rand);

	ElGamal::Ciphertext ctext(coded_m.alpha, coded_m.beta, pk);
	ElGamal::Plaintext ptext = ElGamal::decrypt(sk, ctext);
	ptext.getM(m);
	sm = string(mpz_get_str(NULL, 10, m));
	EXPECT_EQ(0, sm.compare("134"));
}
