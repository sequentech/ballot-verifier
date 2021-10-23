# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

# include GoogleTest library
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
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

add_executable(agora_airgap_tests tests.cpp)

# NOTE: we are not using target_include_directories() for cryptopp because it
# does not support CMake by default and we're using nix anyway to add all the
# libraries in the include path

target_link_libraries(agora_airgap_tests PRIVATE agora-airgap-lib stdc++
                                                 gtest_main cryptopp)

include(GoogleTest)
gtest_discover_tests(agora_airgap_tests)
