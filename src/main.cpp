#include <iostream>

#include "encrypt.h"
#include "sha256.h"

using namespace rapidjson;


int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "You need to supply more arguments. Example:" << argv[0] << " <file_with_auditable_ballot.json>" << endl;
  }
	std::cout << audit(argv[1]) << std::endl;
	return 0;
}