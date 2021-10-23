// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Based on Eduardo Robles's agora-api:
// https://github.com/agoravoting/agora-api

#ifndef ENCRYPT_H
#define ENCRYPT_H
#include <sstream>
#include <string>

#include "common.h"

using namespace std;

namespace AgoraAirgap {

void encrypt_ballot(
    stringstream & out, const string & votes_path, const string & pk_path,
    const string & ballot_path);

void download_audit(stringstream & out, const string & auditable_ballot_path);

void download_audit_text(
    stringstream & out, const string & auditable_ballot_path);
void download(
    stringstream & out, const string & auditable_ballot_path,
    const string & election_path);

void audit(
    stringstream & out, const string & auditable_ballot_path,
    const string & election_path);

}  // namespace AgoraAirgap

#endif  // ENCRYPT_H