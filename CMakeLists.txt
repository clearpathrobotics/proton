cmake_minimum_required(VERSION 3.20)
project(proton LANGUAGES C CXX VERSION 0.0.0)

# Feature flags
option(PROTON_BUILD_TESTS "Build unit tests" ON)
option(PROTON_ENABLE_TEST_COVERAGE "Enable code coverage instrumentation" OFF)
option(PROTON_ENABLE_ALLOC "Enable heap allocation and exceptions" OFF)
option(PROTON_NODE_BUILDER "Enable optional runtime node generation from config file" OFF)
option(PROTON_NODE_BUILDER_YAML_PARSER "Enable optional yaml config file parsing" OFF)
option(PROTON_NODE_BUILDER_JSON_PARSER "Enable optional json file parsing" OFF)
option(PROTON_INSTALL "Install proton library" OFF)
option(PROTON_GENERATE_PROTOS "Regenerate nanopb proto files (requires protoc + Python)" OFF)
set(PROTON_MAX_PENDING_TRIGGERS 4 CACHE STRING "Number of allowable pending triggers in node_manager")

# Code coverage instrumentation option
if (PROTON_BUILD_TESTS)
  enable_testing()
  add_compile_options(-Werror -Wall -Wextra)
  if(PROTON_ENABLE_TEST_COVERAGE)
    message(STATUS "Code coverage instrumentation enabled for proton_core")
    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
  endif()
elseif(PROTON_ENABLE_TEST_COVERAGE)
   message(ERROR "Testing must be enabled to build test coverage")
endif()

# protobuf v*.23.0+ will fail to find Abseil, which is a required dependency.
# Using pkg-config to find protobuf config information instead of direct find_package, to get correct dependencies
find_package(PkgConfig REQUIRED)
if(PkgConfig_FOUND)
  pkg_search_module(Protobuf protobuf CONFIG REQUIRED)
endif()

find_package(Protobuf REQUIRED)

if (Protobuf_VERSION_MINOR GREATER_EQUAL 23)
  find_package(absl CONFIG QUIET)
  if(NOT absl_FOUND)
    message(WARNING "Abseil not found via CMake config, trying pkg-config")
    if(PkgConfig_FOUND)
      pkg_search_module(Abseil absl)
    endif()
  endif()
endif()

add_subdirectory(core)
add_subdirectory(cpp)

# Add coverage compiler and linker flags
if(PROTON_ENABLE_TEST_COVERAGE)
  message(STATUS "Code coverage instrumentation enabled")
  add_compile_options(-fprofile-arcs -ftest-coverage)
  add_link_options(--coverage)
endif()

# Export targets from libraries
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
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/protonCoreGenerator.cmake
    DESTINATION lib/cmake/proton
  )
endif()
