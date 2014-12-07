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

void encrypt_ballot(const std::string & votes_path, const std::string & pk_path, const std::string & ballot_path);
void download_audit(const std::string & auditable_ballot_path);
void download(const std::string & auditable_ballot_path, const std::string & pk_path, const std::string &  voting_options_path);
void audit(const std::string & auditable_ballot_path, const std::string & pk_path, const std::string &  voting_options_path);

#endif //ENCRYPT_H