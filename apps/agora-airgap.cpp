//============================================================================
// Name        : agora-airgap.cpp
// Author      : F�lix Robles felrobelv at gmail dot com
// Version     :
// Copyright   : GPL Affero v3
// Description : Agora Airgap project main file
//============================================================================

#include <iostream>
#include "Agora.h"
using namespace std;


int main() {
	mpz_t pie;
	mpz_init_set_str (pie, "3141592653589793238462643383279502884", 10);
	string s(mpz_get_str(NULL, 10, pie));
	cout << "!!!Hello World!!!" << endl;
	//cout << s << endl;
	mpz_t out;
	mpz_init(out);
	Random::getRandomInteger(out, pie); 
	cout << mpz_get_str(NULL, 10, out) << endl;
	return 0;
}
