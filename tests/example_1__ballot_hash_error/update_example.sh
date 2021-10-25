#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

function edit_file ()
{
    JQ_TEXT=$1
    JQ_FILE=$2
    tmp=$(mktemp)
    jq "${JQ_TEXT}" "${JQ_FILE}" > "$tmp" && mv "$tmp" "${JQ_FILE}"
}

function reset_files ()
{
    BASE_DIR=$1
    FILES=$(ls "${BASE_DIR}")
    for FILE in ${FILES}
    do
        [ -f "${FILE}" ] && rm "${FILE}"
    done
}

function copy_files ()
{
    BASE_DIR=$1
    cp -rf ${BASE_DIR}/* .
}

BASE_DIR="../example_1"
# Execute all the updates for this example
reset_files "${BASE_DIR}"
copy_files "${BASE_DIR}"

edit_file '.ballot_hash="badhash"' 'ballot.json'
edit_file '.ballot_hash="badhash"' 'ballot.json'