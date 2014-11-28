#include <iostream>

#include "encrypt.h"
#include "sha256.h"

using namespace rapidjson;


int main() {
	std::cout << encrypt("pk_1", "votes.json") << std::endl;
	return 0;
}