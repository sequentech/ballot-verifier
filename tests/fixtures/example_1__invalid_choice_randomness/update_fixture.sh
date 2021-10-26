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

edit_file '.choices[1].randomness="7424690150050326870443048709470286345806301329478116429940273303726250019825141393385662798060436512984515952701889710037567439099890408863021052748366603776018976162621587517533318174541911662709294926510665947226450774152482879969684058784708614292221875675611440636381475170288221315762266199188118722964279206718533475845310825071611052939464576217420959662346423154381866615696696403986057399647692493993075179707403054964381662899549087613642180564100648101287436297150858611107113975955016037868922375380338509713529488703697987559060910005044090830784810345751217923171307938879695152908953135937357959301597"' 'ballot.json'