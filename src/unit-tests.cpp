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

TEST(Sha256Test, StringOne) {
	BYTE text1[] = {"abc"};
	BYTE hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
	                                 0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
	BYTE buf[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, text1, sizeof(text1)-1);
	sha256_final(&ctx, buf);
	EXPECT_EQ(0, memcmp(hash1, buf, SHA256_BLOCK_SIZE));
}


TEST(Sha256Test, StringTwo) {
	BYTE text2[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
	BYTE hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
	                                 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	BYTE buf[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, text2, sizeof(text2)-1);
	sha256_final(&ctx, buf);
	EXPECT_EQ(0, memcmp(hash2, buf, SHA256_BLOCK_SIZE));
}

TEST(Sha256Test, StringLoopTest) {
	BYTE text3[] = {"aaaaaaaaaa"};
	BYTE hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
	                                 0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0};
	BYTE buf[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	for (int idx = 0; idx < 100000; ++idx) {
	   sha256_update(&ctx, text3, sizeof(text3)-1);
	}
	sha256_final(&ctx, buf);
	EXPECT_EQ(0, memcmp(hash3, buf, SHA256_BLOCK_SIZE));
}

TEST(ElGamalUnitTest, SmallMessageDecryption) {
	mpz_t p, q, g, y, x, m,  alpha, beta;
	string sm;
	
	mpz_init_set_str(p, "1019", 10);
	mpz_init_set_str(m, "0", 10);
	mpz_init_set_str(q, "359", 10); //q = (p-1)/2 = 359
	mpz_init_set_str(g, "3", 10);
	mpz_init_set_str(y, "770", 10); //y = (g^x) mod p
	mpz_init_set_str(alpha, "81", 10);
	mpz_init_set_str(beta, "818", 10);
	mpz_init_set_str(x, "66", 10);
	ElGamal::PublicKey pk(p, q, g, y);
	ElGamal::Ciphertext ctext(alpha, beta, pk);
	ElGamal::SecretKey sk(x, pk);
	ElGamal::Plaintext ptext = ElGamal::decrypt(sk, ctext);
	ptext.getM(m);
	sm = string(mpz_get_str(NULL, 10, m));
	EXPECT_EQ(0, sm.compare("134"));
	
	//U = r^x mod p = (81^66) mod 1019 = 508
	//U^-1  <----------> (U^-1) U  = 1 mod p -> (U^-1) U = 1 + p*n              so U^-1 = 339
	//M = (U^-1) t  mod p = (339 * 818) mod 1019 = 134
}

TEST(AgoraUnitTest, SmallMessageEncryption) {
	
	mpz_t p, q, g, y, x, m, rand;
	string alpha, beta;
	
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
	//Random::getRandomInteger(rand, pk.q);
	mpz_init_set_str(rand, "4", 10);
	//alpha = (g^random) mod p  = (3^4) mod 1019 = 81
	//U = (h^random) mod p = (770^4) mod 1019 = 508
	//M = m+1 = 134
	//beta = ( U M  ) mod p = ( 508 * 134) mod 1019 = 818
	Agora::Encrypted_answer coded_m = Agora::encryptAnswer(pk, m, rand);
	alpha = mpz_get_str(NULL, 10, coded_m.alpha);
	beta = mpz_get_str(NULL, 10, coded_m.beta);
	EXPECT_EQ(0, alpha.compare("81"));
	EXPECT_EQ(0, beta.compare("818"));
}


