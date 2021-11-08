#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

# Execute all the updates for the examples
FIXTURE_SUBDIRS=$(ls -d fixtures/*)
CURRENT_DIR=$PWD
echo -e "<dirs>\n${FIXTURE_SUBDIRS}\n</dirs>\n"
for FIXTURE_SUBDIR in ${FIXTURE_SUBDIRS}
do
    [ -d ${CURRENT_DIR}/${FIXTURE_SUBDIR} ] || continue
    echo -e "cd \"${CURRENT_DIR}/${FIXTURE_SUBDIR}\""
    cd "${CURRENT_DIR}/${FIXTURE_SUBDIR}"
    [ -f ./update_fixture.sh ] && ./update_fixture.sh
done