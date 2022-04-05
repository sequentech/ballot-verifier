#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

source ../common.sh

BASE_DIR="../example_1"
rm_files
copy_files "${BASE_DIR}"

edit_file '.choices[0].plaintext="2"' 'auditable_ballot.json'