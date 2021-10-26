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

edit_file '.choices[0].alpha="9889833205843920213555407568922422020272667597164432610876157173469327533028419724717294427279318834056111483971412879713811420280400235762200017964625138033250814077878006932705996485701725883573752120663584385317836384707975483016270988935508694936207808864473075609234754529947389956710309403448615146917408284905873638666259944375861930090500745785057939588587092129513863334843510001759348808844792263864424735682176740134735268982986046274196466417988604828860007798597331796655512295591366128919310093935872043651894764953584624980432346389610487095864802560958191011484577407791137612169265930399403743587970"' 'ballot.json'