# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

# include PkgConfig for dependency management
include(FindPkgConfig)

# depend on GMP and GMPXX
find_package(GMP REQUIRED)
find_package(GMPXX REQUIRED)

# Depend on libcurl. How to:
# https://cmake.org/cmake/help/latest/module/FindCURL.html
pkg_check_modules(CURL libcurl REQUIRED)

# Depend on wxWidgets. How to: https://wiki.wxwidgets.org/CMake
find_package(
  wxWidgets
  COMPONENTS core base
  REQUIRED)
include("${wxWidgets_USE_FILE}")

# NOTE: we also depend on rapidjson, but we are letting nix take care of that

# TODO: does not locate headers correctly in nix Code Quality: ensure clang-tidy
# is used to inspect code quality set(CMAKE_CXX_CLANG_TIDY
# "clang-tidy;-checks=*")

# Create the common library #################################################

# TODO: move header files to a include dir
file(GLOB SEQUENT_AIRGAP_HEADER_LIST CONFIGURE_DEPENDS "*.h")

add_library(
  ballot-verifier-lib sha256.cpp ElGamal.cpp Random.cpp encrypt.cpp MixedRadix.cpp
                           BallotCodec.cpp ${SEQUENT_AIRGAP_HEADER_LIST})

# All users of this library will need at least C++11
target_compile_features(ballot-verifier-lib PUBLIC cxx_std_11)

target_include_directories(
  ballot-verifier-lib PRIVATE ${GMP_INCLUDES} ${GMPXX_INCLUDES}
                           ${CURL_INCLUDE_DIRS})

# We need this directory in the and users of this library and users of the
# library needs them too
target_include_directories(
  ballot-verifier-lib
  PUBLIC "../include" SYSTEM
  PUBLIC "../include")

target_link_libraries(
  ballot-verifier-lib PRIVATE stdc++ ${GMP_LIBRARIES} ${GMPXX_LIBRARIES}
                           ${CURL_LIBRARIES})

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${SEQUENT_AIRGAP_HEADER_LIST})
