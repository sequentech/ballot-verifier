# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@sequentech.io>
#
# SPDX-License-Identifier: AGPL-3.0-only

# include GoogleTest library (latest version, v1.11.0)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)

# GoogleTest: For Windows: Prevent overriding the parent project's
# compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# TODO: does not locate headers correctly in nix Code Quality: ensure clang-tidy
# is used to inspect code quality set(CMAKE_CXX_CLANG_TIDY
# "clang-tidy;-checks=*")

add_executable(ballot_verifier_tests tests.cpp TestMixedRadix.cpp
                                                 TestBallotCodec.cpp)

# NOTE: we are not using target_include_directories() for cryptopp because it
# does not support CMake by default and we're using nix anyway to add all the
# libraries in the include path

target_link_libraries(ballot_verifier_tests PRIVATE ballot-verifier-lib stdc++
                                                 gtest_main gmock cryptopp)

include(GoogleTest)
gtest_discover_tests(ballot_verifier_tests)
