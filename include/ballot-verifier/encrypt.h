// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
//
// SPDX-License-Identifier: AGPL-3.0-only

// Based on Eduardo Robles's sequent-api:
// https://github.com/sequent/sequent-api

#ifndef ENCRYPT_H
#define ENCRYPT_H
#include <functional>
#include <sstream>
#include <string>

#include "common.h"

using std::function;
using std::string;
using std::stringstream;

namespace BallotVerifier {

typedef function<string(stringstream &, const string &)> DownloadFunc;

void encrypt_ballot(
    stringstream & out,
    const string & votes_path,
    const string & pk_path,
    const string & ballot_path);

string download_url(stringstream & out, const string & url);
string read_file(stringstream & out, const string & path);

void download_audit(
    stringstream & out,
    const string & auditable_ballot_path,
    const DownloadFunc & download_func = &download_url);

void download_audit_text(
    stringstream & out,
    const string & auditable_ballot_path,
    const DownloadFunc & download_func = &download_url);

void download(
    stringstream & out,
    const string & auditable_ballot_path,
    const string & election_path,
    const DownloadFunc & download_func = &download_url);

void audit(
    stringstream & out,
    const string & auditable_ballot_path,
    const string & election_path);

}  // namespace BallotVerifier

#endif  // ENCRYPT_H