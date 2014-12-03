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

std::string encrypt(const std::string & pk_path, const std::string & votes_path);
std::string audit(const std::string & auditable_ballot_path);

#endif //ENCRYPT_H