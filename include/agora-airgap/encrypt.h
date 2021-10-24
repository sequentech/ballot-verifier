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
#include <functional>

#include "common.h"

using std::stringstream;
using std::string;
using std::function;

namespace AgoraAirgap {

typedef function<string(stringstream &, const string &)> DownloadFunc;

void encrypt_ballot(
    stringstream & out,
    const string & votes_path,
    const string & pk_path,
    const string & ballot_path);

string download_url(stringstream & out, const string & url);

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

}  // namespace AgoraAirgap

#endif  // ENCRYPT_H