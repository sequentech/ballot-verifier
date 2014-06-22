#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <cryptopp/elgamal.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>

#include <gtest/gtest.h>


bool func() {
	CryptoPP::ElGamal::Decryptor* decryptor = NULL;
	return true;
}

// Testing google test and using CryptoPP
TEST(JustATest, Zero) {
  EXPECT_TRUE(func());
}

