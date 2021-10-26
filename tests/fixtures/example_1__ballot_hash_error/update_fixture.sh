#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

function not_in()
{
    array=$1
    search=$2
    for i in "${array[@]}"
    do
        if [ "$i" == "$search" ] ; then
            return 1
        fi
    done
    return 0
}

function edit_file ()
{
    JQ_TEXT=$1
    JQ_FILE=$2
    tmp=$(mktemp)
    jq "${JQ_TEXT}" "${JQ_FILE}" > "$tmp" && mv "$tmp" "${JQ_FILE}"
}

# Remove all files.. except the exceptions
function rm_files ()
{
    echo "rm_files"
    BASE_DIR=$1
    EXCEPTIONS=$2
    FILES=$(ls "${BASE_DIR}")
    for FILE in ${FILES}
    do
        if not_in $EXCEPTIONS $FILE
        then
            if [ -f "${FILE}" ] || [ -d "${FILE}" ]
            then
                echo "($PWD) rm -rf \"${FILE}\""
                rm -rf "${FILE}"
            fi
        fi
    done
}

# Copy all files.. except the exceptions
function copy_files ()
{
    echo "copy_files"
    BASE_DIR=$1
    EXCEPTIONS=$2
    FILES=$(ls "${BASE_DIR}")
    for FILE in ${FILES}
    do
        if not_in $EXCEPTIONS $FILE
        then
            echo "($PWD) cp -rf \"${BASE_DIR}/${FILE}\" ."
            cp -rf "${BASE_DIR}/${FILE}" .
        fi
    done
}

BASE_DIR="../example_1"
EXCEPTIONS=("expectations.json")
rm_files "${BASE_DIR}" $EXCEPTIONS
copy_files "${BASE_DIR}" $EXCEPTIONS

edit_file '.ballot_hash="badhash"' 'ballot.json'