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

edit_file '.choices[0].alpha="9046045967271876135779162825596454535487788165677432795915538644643873019294972075425068011370196538257963274087919446754442827754936753170901781811177153140046309813477539604693341665786445496531785949782261131427043724158709301288478465982005768683635508427316050549648122274266041863260503236786157817921497157855499843399654006283088196097893906803039500406767337743571810857583229258217407313822126790018628679829918629181064418908228966803190442472671271824694648478661038919883769664886006500773416451454379516164479636832283032472326259923116064652371604745283189764246015063125981653283715559254370485084520"' 'ballot.json'