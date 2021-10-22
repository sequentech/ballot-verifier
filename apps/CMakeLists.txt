# include PkgConfig for dependency management
include(FindPkgConfig)

# Depend on wxWidgets. How to: https://wiki.wxwidgets.org/CMake
find_package(
  wxWidgets
  COMPONENTS core base
  REQUIRED)
include("${wxWidgets_USE_FILE}")

# Create the agora-airgap-gui binary ########################################

add_executable(agora-airgap-gui interface.cpp interface.h)

target_link_libraries(agora-airgap-gui PRIVATE agora-airgap-lib stdc++
                                               ${wxWidgets_LIBRARIES})

install(TARGETS agora-airgap-gui DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)

# Create the agora-airgap command line utility binary #######################

add_executable(agora-airgap main.cpp)

target_link_libraries(agora-airgap PRIVATE agora-airgap-lib stdc++)

install(TARGETS agora-airgap DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
