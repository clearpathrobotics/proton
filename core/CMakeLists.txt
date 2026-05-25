cmake_minimum_required(VERSION 3.14)
project(proton_core C CXX)

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

include(FetchContent)
# -------------------------------
# Fetch nanopb (runtime + generator)
# -------------------------------
set(nanopb_BUILD_GENERATOR ON CACHE BOOL "Build nanopb generator" FORCE)
FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG        0.4.9.1
  SOURCE_SUBDIR  null
)
FetchContent_MakeAvailable(nanopb)

# Protobuf compiler
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()

# proton_core generator

set(PROTON_CORE_GENERATOR ${CMAKE_CURRENT_LIST_DIR}/generator_scripts/generator.py)

function(proton_core_generator GENERATED_FILES GENERATED_FOLDER TARGET)
  # CONFIG_FILE is optional - use PROTON_CORE_CONFIG_FILE if set by user
  set(options "")
  set(oneValueArgs CONFIG_FILE)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_CONFIG_FILE)
    set(CONFIG_FILE ${ARG_CONFIG_FILE})
  elseif(PROTON_CORE_CONFIG_FILE)
    set(CONFIG_FILE ${PROTON_CORE_CONFIG_FILE})
  endif()

  # Build optional config argument
  set(CONFIG_ARG "")
  set(CONFIG_DEPENDS "")
  if(CONFIG_FILE)
    set(CONFIG_ARG -c ${CONFIG_FILE})
    if(EXISTS ${CONFIG_FILE})
      set(CONFIG_DEPENDS ${CONFIG_FILE})
    endif()
  endif()

  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(PROTON_CORE_GENERATOR ${CMAKE_CURRENT_LIST_DIR}/generator_scripts/generator.py)
  # Add a custom target to execute the script
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E env
      PYTHONPATH=${CMAKE_CURRENT_LIST_DIR}/generator_scripts
      ${Python3_EXECUTABLE}
      ${PROTON_CORE_GENERATOR}
      ${CONFIG_ARG}
      -d ${GENERATED_FOLDER}
      -t ${TARGET}
    DEPENDS ${CONFIG_DEPENDS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python script: ${Python3_EXECUTABLE} ${PROTON_CORE_GENERATOR} ${CONFIG_ARG} -d ${GENERATED_FOLDER} -t ${TARGET}"
  )
endfunction()

# -------------------------------
# Proto files
# -------------------------------
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_FILES
  ${PROTO_PATH}/signal.proto
  ${PROTO_PATH}/bundle.proto
  ${PROTO_PATH}/proton.proto
)

set(PROTO_OPTION_FILES
  ${PROTO_PATH}/signal.options
)

set(PROTO_DEPENDS ${PROTO_FILES} ${PROTO_OPTION_FILES})

# Generated sources go into build dir
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(GENERATED_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME})
set(GENERATED_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
file(MAKE_DIRECTORY ${GENERATED_DIR})
file(MAKE_DIRECTORY ${GENERATED_INC_DIR})
file(MAKE_DIRECTORY ${GENERATED_SRC_DIR})

# -------------------------------
# Custom command to run protoc + nanopb generator
# -------------------------------
set(NANOPB_GENERATED_FILES
  "${GENERATED_DIR}/signal.pb.c"
  "${GENERATED_DIR}/signal.pb.h"
  "${GENERATED_DIR}/bundle.pb.c"
  "${GENERATED_DIR}/bundle.pb.h"
  "${GENERATED_DIR}/proton.pb.c"
  "${GENERATED_DIR}/proton.pb.h"
)

set(NANOPB_GENERATED_SRCS
  "${GENERATED_SRC_DIR}/signal.pb.c"
  "${GENERATED_SRC_DIR}/bundle.pb.c"
  "${GENERATED_SRC_DIR}/proton.pb.c"
)

set(NANOPB_GENERATED_HEADERS
  "${GENERATED_INC_DIR}/signal.pb.h"
  "${GENERATED_INC_DIR}/bundle.pb.h"
  "${GENERATED_INC_DIR}/proton.pb.h"
)

add_custom_command(
  OUTPUT ${NANOPB_GENERATED_FILES}
  COMMAND protoc
    --proto_path=${PROTO_PATH}
    --plugin=protoc-gen-nanopb=${nanopb_SOURCE_DIR}/generator/protoc-gen-nanopb
    --nanopb_out=${GENERATED_DIR} signal.proto bundle.proto proton.proto
    -I${nanopb_SOURCE_DIR}/generator/proto
  DEPENDS ${PROTO_DEPENDS}
  WORKING_DIRECTORY ${PROTO_PATH}
  COMMENT "Generating nanopb files for ${PROTO_NAME}.proto"
)

add_custom_command(
  OUTPUT ${NANOPB_GENERATED_SRCS}
  COMMAND sed -i 's|"bundle.pb.h"|"${PROJECT_NAME}/bundle.pb.h"|' ${GENERATED_DIR}/bundle.pb.c
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/signal.pb.c
  COMMAND sed -i 's|"proton.pb.h"|"${PROJECT_NAME}/proton.pb.h"|' ${GENERATED_DIR}/proton.pb.c
  COMMAND cp ${GENERATED_DIR}/signal.pb.c ${GENERATED_SRC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.c ${GENERATED_SRC_DIR}
  COMMAND cp ${GENERATED_DIR}/proton.pb.c ${GENERATED_SRC_DIR}
  DEPENDS ${PROTO_DEPENDS} ${NANOPB_GENERATED_FILES}
)

add_custom_command(
  OUTPUT ${NANOPB_GENERATED_HEADERS}
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/bundle.pb.h
  COMMAND sed -i 's|"bundle.pb.h"|"${PROJECT_NAME}/bundle.pb.h"|' ${GENERATED_DIR}/proton.pb.h
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/proton.pb.h
  COMMAND cp ${GENERATED_DIR}/signal.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/proton.pb.h ${GENERATED_INC_DIR}
  DEPENDS ${PROTO_DEPENDS} ${NANOPB_GENERATED_FILES}
)

add_custom_target(generate_protoc ALL
  DEPENDS
  ${NANOPB_GENERATED_FILES}
  ${NANOPB_GENERATED_SRCS}
  ${NANOPB_GENERATED_HEADERS}
)

# -------------------------------
# autogen signal registry
# -------------------------------
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)


# -------------------------------
# proton_core library
# -------------------------------
option(PROTON_DEBUG "Enable PROTON_DEBUG preprocessor macro" OFF)

set(PROTON_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(${PROJECT_NAME}
  src/node_manager.c
  src/encode_decode.c
  src/registry.c
  src/node_manager.c
  src/proton_field_callbacks.c
  src/transport/serial.c
  src/transport/udp4.c
  ${nanopb_SOURCE_DIR}/pb_common.c
  ${nanopb_SOURCE_DIR}/pb_decode.c
  ${nanopb_SOURCE_DIR}/pb_encode.c
  ${NANOPB_GENERATED_SRCS}
)

add_dependencies(${PROJECT_NAME} generate_protoc)

if (PROTON_DEBUG)
  target_compile_definitions(proton_core PUBLIC PROTON_DEBUG)
endif()

target_include_directories(proton_core PUBLIC ${GENERATED_FOLDER} ../include)

add_library(proton::proton_core ALIAS proton_core)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${nanopb_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROTON_COMMON_INCLUDE_DIR}>
)

# Install
if (PROTON_INSTALL)
  install(TARGETS proton_core
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
    DESTINATION include/proton_core
  )
endif()

# Testing
enable_testing()
find_package(GTest REQUIRED)

# autogen registry for tests
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
  CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tests/core/test.yaml
)

add_executable(registry_test
  tests/core/registry_test.cpp
  ${GENERATED_REGISTRY_FILES}
)

target_link_libraries(registry_test PUBLIC
  GTest::gtest_main
  proton::proton_core
)

target_include_directories(registry_test PUBLIC
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/core
)

add_executable(encode_decode_test
  tests/core/encode_decode_test.cpp
  ${GENERATED_REGISTRY_FILES}
)

target_link_libraries(encode_decode_test PUBLIC
  GTest::gtest_main
  proton::proton_core
)

target_include_directories(encode_decode_test PUBLIC
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/core
)

add_executable(serial_transport_test
  tests/core/serial_transport_test.cpp
  ${GENERATED_REGISTRY_FILES}
)

target_link_libraries(serial_transport_test PUBLIC
  GTest::gtest_main
  proton::proton_core
)

target_include_directories(serial_transport_test PUBLIC
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/core
)

add_executable(udp4_transport_test
  tests/core/udp4_transport_test.cpp
  ${GENERATED_REGISTRY_FILES}
)

target_link_libraries(udp4_transport_test PUBLIC
  GTest::gtest_main
  proton::proton_core
)

target_include_directories(udp4_transport_test PUBLIC
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/core
)

add_executable(node_manager_test
  tests/core/node_manager_test.cpp
  ${GENERATED_REGISTRY_FILES}
)

target_link_libraries(node_manager_test PUBLIC
  GTest::gtest_main
  proton::proton_core
)

target_include_directories(node_manager_test PUBLIC
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/core
)

include(GoogleTest)
gtest_discover_tests(registry_test)
gtest_discover_tests(encode_decode_test)
gtest_discover_tests(serial_transport_test)
gtest_discover_tests(udp4_transport_test)
gtest_discover_tests(node_manager_test)
