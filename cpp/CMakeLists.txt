cmake_minimum_required(VERSION 3.14)
project(proton_core_cpp C CXX)

# Code coverage instrumentation option
option(PROTON_BUILD_TESTS "Build unit tests" ON)
option(PROTON_ENABLE_TEST_COVERAGE "Enable code coverage instrumentation" OFF)
option(PROTON_ENABLE_ALLOC "Enable heap allocation and exceptions" OFF)

if (PROTON_BUILD_TESTS)
  add_compile_options(-Werror -Wall -Wextra)
  if(PROTON_ENABLE_TEST_COVERAGE)
    message(STATUS "Code coverage instrumentation enabled for proton_core")
    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
  endif()
elseif(PROTON_ENABLE_TEST_COVERAGE)
   message(ERROR "Testing must be enabled to build test coverage")
endif()

set(PROTON_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Add proton_core dependency if not already defined
if(NOT TARGET proton_core)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core ${CMAKE_CURRENT_BINARY_DIR}/core)
endif()

add_library(${PROJECT_NAME}
  src/bundle_access.cpp
  src/signal_access.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROTON_COMMON_INCLUDE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    proton::proton_core
)

if(PROTON_ENABLE_ALLOC)
  target_compile_definitions(${PROJECT_NAME} PUBLIC PROTON_ENABLE_ALLOC=1)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions -fno-rtti)
endif()

add_library(proton::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Testing
if (PROTON_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(${CMAKE_CURRENT_LIST_DIR}/../cmake/protonCoreGenerator.cmake)

  set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/tests/generated)
  set(GENERATED_REGISTRY_FILES
    "${GENERATED_FOLDER}/target_registry.c"
    "${GENERATED_FOLDER}/target_node.c"
    "${GENERATED_FOLDER}/target_registry_ids.h"
    "${GENERATED_FOLDER}/target_registry_sizes.h"
    "${GENERATED_FOLDER}/target_connections.h"
  )

  proton_core_generator(
    "${GENERATED_REGISTRY_FILES}"
    ${GENERATED_FOLDER}
    "producer"
    CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core/test.yaml
  )

  add_executable(registry_test_cpp
    tests/registry_test.cpp
    ${GENERATED_REGISTRY_FILES}
  )

  target_link_libraries(registry_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_core_cpp
  )

  target_include_directories(registry_test_cpp PUBLIC
    ${GENERATED_FOLDER}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  include(GoogleTest)
  gtest_discover_tests(registry_test_cpp)
endif()
