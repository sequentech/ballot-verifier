cmake_minimum_required(VERSION 3.14)

# Project name and a few useful settings. Other commands can pick up the results
project(
    AgoraAirgap
    VERSION 4.0.2
    DESCRIPTION "cast-as-intended verifier for nVotes platform"
    LANGUAGES CXX
)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

# allow our custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
#if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
#   AND BUILD_TESTING)
#  add_subdirectory(tests)
#endif()
