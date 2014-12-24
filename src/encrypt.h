/*
 * Encrypt.h
 *
 *  Created on: November 19, 2014
 *      Author: Félix Robles felrobelv at gmail dot com
 * Based on Eduardo Robles's agora-api:
 * https://github.com/agoravoting/agora-api
 */
#ifndef ENCRYPT_H
#define ENCRYPT_H
#include "common.h"
#include <string>
#include <sstream>

using namespace std;

void encrypt_ballot(stringstream& out, const string & votes_path, const string & pk_path, const string & ballot_path);
void download_audit(stringstream& out, const string & auditable_ballot_path);
void download_audit_text(stringstream& out, const string & auditable_ballot_path);
void download(stringstream& out, const string & auditable_ballot_path, const string &  election_path);
void audit(stringstream& out, const string & auditable_ballot_path, const string & election_path);

#endif //ENCRYPT_H