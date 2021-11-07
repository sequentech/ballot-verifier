#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

source ../common.sh

BASE_DIR="../example_1"
rm_files
copy_files "${BASE_DIR}"

echo -e "erfewr,\"f#\n{," >> auditable_ballot.json