
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Agora.h"

using namespace std;


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	return 0;
}
