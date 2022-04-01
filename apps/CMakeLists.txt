# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

# include PkgConfig for dependency management
include(FindPkgConfig)

# Depend on wxWidgets. How to: https://wiki.wxwidgets.org/CMake
find_package(
  wxWidgets
  COMPONENTS core base
  REQUIRED)
include("${wxWidgets_USE_FILE}")

# Create the ballot-verifier-gui binary ########################################

add_executable(ballot-verifier-gui interface.cpp interface.h)

target_link_libraries(ballot-verifier-gui PRIVATE ballot-verifier-lib stdc++
                                               ${wxWidgets_LIBRARIES})

install(TARGETS ballot-verifier-gui DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)

# Create the ballot-verifier command line utility binary #######################

add_executable(ballot-verifier main.cpp)

target_link_libraries(ballot-verifier PRIVATE ballot-verifier-lib stdc++)

install(TARGETS ballot-verifier DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
