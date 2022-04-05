# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

if(GMPXX_INCLUDES AND GMPXX_LIBRARIES)
  set(GMPXX_FIND_QUIETLY TRUE)
endif(GMPXX_INCLUDES AND GMPXX_LIBRARIES)

find_path(
  GMPXX_INCLUDES
  NAMES gmpxx.h
  PATHS $ENV{GMPDIR} ${INCLUDE_INSTALL_DIR})

find_library(GMPXX_LIBRARIES gmpxx PATHS $ENV{GMPDIR} ${LIB_INSTALL_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMPXX DEFAULT_MSG GMPXX_INCLUDES
                                  GMPXX_LIBRARIES)
mark_as_advanced(GMPXX_INCLUDES GMPXX_LIBRARIES)
