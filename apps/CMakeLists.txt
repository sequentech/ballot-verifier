# include PkgConfig for dependency management
include(FindPkgConfig)

# Depend on wxWidgets. 
# How to: https://wiki.wxwidgets.org/CMake
find_package(wxWidgets COMPONENTS core base REQUIRED)
include("${wxWidgets_USE_FILE}")


#### Create the agora-airgap binary ############################################

add_executable(
    agora-airgap 
    interface.cpp
    interface.h
)

target_link_libraries(
    agora-airgap 
    PRIVATE 
    agora-airgap-lib
    stdc++
    ${wxWidgets_LIBRARIES} 
)

install(
    TARGETS
    agora-airgap
    DESTINATION
    ${CMAKE_INSTALL_PREFIX}/bin/
)