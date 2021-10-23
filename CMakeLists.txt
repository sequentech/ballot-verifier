# SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
#
# SPDX-License-Identifier: AGPL-3.0-only

cmake_minimum_required(VERSION 3.14)

# Project name and a few useful settings. Other commands can pick up the results
project(
  AgoraAirgap
  VERSION 4.0.2
  DESCRIPTION "cast-as-intended verifier for nVotes platform"
  LANGUAGES CXX)

include(cmake/CPM.cmake)

cpmaddpackage(
  NAME
  Format.cmake
  VERSION
  1.7.0
  GITHUB_REPOSITORY
  TheLartians/Format.cmake
  OPTIONS
  # set to yes skip cmake formatting
  "FORMAT_SKIP_CMAKE NO"
  # path to exclude (optional, supports regular expressions)
  "CMAKE_FORMAT_EXCLUDE (cmake/.*|_deps/.*|cpm-package-lock\.cmake|CMakeLists.txt)"
)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

# Code Quality: Enable all the compiler warnings and ensure to error on
# warnings.
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# allow our custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")

# Only do these if this is the main project, and not if it is included through
# add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

option(FORCE_COLORED_OUTPUT
       "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(${FORCE_COLORED_OUTPUT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)

include(CTest)
enable_testing()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
