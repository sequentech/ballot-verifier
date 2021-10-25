#!/bin/bash
# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

# Execute all the updates for the examples
EXAMPLE_SUBDIRS=$(ls -d example*__*/)
CURRENT_DIR=$PWD
for EXAMPLE_SUBDIR in "${EXAMPLE_SUBDIRS}"
do
    cd "${CURRENT_DIR}/${EXAMPLE_SUBDIR}"
    ./update_example.sh
done