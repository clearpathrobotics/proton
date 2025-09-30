cmake_minimum_required(VERSION 3.28)
project(proton LANGUAGES C CXX VERSION 0.0.0)

if(NOT CMAKE_CROSSCOMPILING)
  find_package(Protobuf REQUIRED)
endif()

add_subdirectory(protonc)
add_subdirectory(protoncpp)
add_subdirectory(examples)

# Export targets from both libraries
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/protonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/protonConfig.cmake
    INSTALL_DESTINATION lib/cmake/proton
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/protonConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    EXPORT ProtonTargets
    NAMESPACE proton::
    DESTINATION lib/cmake/proton
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/protonConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/protonConfigVersion.cmake
    DESTINATION lib/cmake/proton
)