cmake_minimum_required(VERSION 3.20)
project(proton LANGUAGES C CXX VERSION 0.0.0)

if(NOT CMAKE_CROSSCOMPILING)
  # Protobuf
  # protobuf v22+ will fail to find Abseil, which is a required dependency.
  # Using pkg-config to find protobuf config information instead of direct find_package, to get correct depdendencies
  find_package(PkgConfig REQUIRED)
  if(PkgConfig_FOUND)
    pkg_search_module(Protobuf protobuf CONFIG REQUIRED)
  endif()

  find_package(Protobuf REQUIRED)

  # Find Abseil separately - required for protobuf v22+
  find_package(absl CONFIG QUIET)
  if(NOT absl_FOUND)
    message(WARNING "Abseil not found via CMake config, trying pkg-config")
    if(PkgConfig_FOUND)
      pkg_search_module(Abseil absl)
    endif()
  endif()

  message (WARNING "PROTOBUF_LIBRARIES ${PROTOBUF_LIBRARIES}")
endif()

option(PROTON_BUILD_PROTONC "Build protonc library" ON)
option(PROTON_BUILD_PROTONCPP "Build protoncpp library" ON)
option(PROTON_BUILD_EXAMPLES "Build proton examples" ON)
option(PROTON_BUILD_TESTS "Build proton tests" ON)
option(PROTON_INSTALL "Install proton" ON)

if (PROTON_BUILD_PROTONC)
  add_subdirectory(protonc)
endif()

if (PROTON_BUILD_PROTONCPP)
  add_subdirectory(protoncpp)
endif()

if (PROTON_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (PROTON_BUILD_TESTS)
  add_subdirectory(tests)
endif()

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

if (PROTON_INSTALL)
  install(
      EXPORT ProtonTargets
      NAMESPACE proton::
      DESTINATION lib/cmake/proton
  )

  install(
      FILES
          ${CMAKE_CURRENT_BINARY_DIR}/protonConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/protonConfigVersion.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/protonFunctions.cmake
      DESTINATION lib/cmake/proton
  )
endif()
