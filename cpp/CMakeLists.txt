cmake_minimum_required(VERSION 3.14)
project(proton_cpp C CXX)

set(PROTON_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Add proton_core dependency if not already defined
if(NOT TARGET proton_core)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core ${CMAKE_CURRENT_BINARY_DIR}/core)
endif()

add_library(${PROJECT_NAME}
  src/bundle_access.cpp
  src/signal_access.cpp
  src/node_builder/config.cpp
  src/node_builder/config_tree.cpp
  src/node_builder/generator.cpp
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

if (PROTON_NODE_BUILDER_JSON_PARSER OR PROTON_NODE_BUILDER_YAML_PARSER)
  if (NOT PROTON_NODE_BUILDER)
    message(ERROR "PROTON_NODE_BUILDER must be enabled for PROTON_NODE_BUILDER_JSON_PARSER or PROTON_NODE_BUILDER_YAML_PARSER")
  endif()
endif()

if (PROTON_NODE_BUILDER)
  if (NOT PROTON_ENABLE_ALLOC)
    message(ERROR "PROTON_ENABLE_ALLOC must be enabled for PROTON_NODE_BUILDER")
  endif()

  target_compile_definitions(${PROJECT_NAME} PUBLIC PROTON_NODE_BUILDER=1)

  if (PROTON_NODE_BUILDER_JSON_PARSER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PROTON_NODE_BUILDER_JSON_PARSER=1)
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
  endif()

  if (PROTON_NODE_BUILDER_YAML_PARSER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PROTON_NODE_BUILDER_YAML_PARSER=1)
    find_package(yaml-cpp REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)
  endif()
endif()

add_library(proton::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (PROTON_INSTALL)
  install(TARGETS ${PROJECT_NAME}
    EXPORT ProtonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

  install(DIRECTORY
    include/
    DESTINATION include
  )

  install(DIRECTORY
    ${GENERATED_INC_DIR}/
    DESTINATION include/protoncpp
  )

  install(
    FILES
      ${CMAKE_CURRENT_LIST_DIR}/../cmake/protonConfig.cmake
      ${CMAKE_CURRENT_LIST_DIR}/../cmake/protonConfigVersion.cmake
      ${CMAKE_CURRENT_LIST_DIR}/../cmake/protonCoreGenerator.cmake
    DESTINATION lib/cmake/proton
  )
endif()

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
    proton::proton_cpp
  )

  target_include_directories(registry_test_cpp PUBLIC
    ${GENERATED_FOLDER}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(lock_test_cpp
    tests/lock_test.cpp
  )

  target_link_libraries(lock_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(lock_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(node_manager_test_cpp
    tests/node_manager_test.cpp
    ${GENERATED_REGISTRY_FILES}
  )

  target_link_libraries(node_manager_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(node_manager_test_cpp PUBLIC
    ${GENERATED_FOLDER}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(serial_transport_test_cpp
    tests/serial_transport_test.cpp
  )

  target_link_libraries(serial_transport_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(serial_transport_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(udp4_transport_test_cpp
    tests/udp4_transport_test.cpp
  )

  target_link_libraries(udp4_transport_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(udp4_transport_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(node_builder_config_test_cpp
    tests/node_builder_config_test.cpp
  )

  target_link_libraries(node_builder_config_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(node_builder_config_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(node_builder_config_tree_test_cpp
    tests/node_builder_config_tree_test.cpp
  )

  target_link_libraries(node_builder_config_tree_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(node_builder_config_tree_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(node_builder_generator_test_cpp
    tests/node_builder_generator_test.cpp
  )

  target_link_libraries(node_builder_generator_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(node_builder_generator_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  add_executable(node_builder_generator_round_trip_test_cpp
    tests/node_builder_generator_round_trip_test.cpp
  )

  target_link_libraries(node_builder_generator_round_trip_test_cpp PUBLIC
    GTest::gtest_main
    proton::proton_cpp
  )

  target_include_directories(node_builder_generator_round_trip_test_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/tests/core
  )

  include(GoogleTest)
  gtest_discover_tests(registry_test_cpp)
  gtest_discover_tests(lock_test_cpp)
  gtest_discover_tests(node_manager_test_cpp)
  gtest_discover_tests(serial_transport_test_cpp)
  gtest_discover_tests(udp4_transport_test_cpp)
  gtest_discover_tests(node_builder_config_test_cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )
  gtest_discover_tests(node_builder_config_tree_test_cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )
  gtest_discover_tests(node_builder_generator_test_cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )
  gtest_discover_tests(node_builder_generator_round_trip_test_cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )
endif()
