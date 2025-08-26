cmake_minimum_required(VERSION 3.14)
project(protoncpp CXX)

# Protobuf compiler
find_package(Protobuf REQUIRED)
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()

# YAML-CPP
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)

# -------------------------------
# Proto files
# -------------------------------
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(PROTO_FILES
    ${PROTO_PATH}/signal.proto
    ${PROTO_PATH}/bundle.proto
)

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
set(GENERATED_FILES
  "${GENERATED_DIR}/signal.pb.cc"
  "${GENERATED_DIR}/signal.pb.h"
  "${GENERATED_DIR}/bundle.pb.cc"
  "${GENERATED_DIR}/bundle.pb.h"
)

set(GENERATED_SRCS
  "${GENERATED_SRC_DIR}/signal.pb.cc"
  "${GENERATED_SRC_DIR}/bundle.pb.cc"
)

set(GENERATED_HEADERS
  "${GENERATED_INC_DIR}/signal.pb.h"
  "${GENERATED_INC_DIR}/bundle.pb.h"
)

add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND protoc
        --proto_path=${PROTO_PATH}
        --cpp_out=${GENERATED_DIR} signal.proto bundle.proto
    DEPENDS  ${PROTO_FILES}
    WORKING_DIRECTORY ${PROTO_PATH}
    COMMENT "Generating cpp protobuf files for ${PROTO_NAME}.proto"
)

add_custom_command(
  OUTPUT ${GENERATED_SRCS}
  COMMAND sed -i 's|"bundle.pb.h"|"${PROJECT_NAME}/bundle.pb.h"|' ${GENERATED_DIR}/bundle.pb.cc
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/signal.pb.cc
  COMMAND cp ${GENERATED_DIR}/signal.pb.cc ${GENERATED_SRC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.cc ${GENERATED_SRC_DIR}
  DEPENDS ${GENERATED_DIR}
)

add_custom_command(
  OUTPUT ${GENERATED_HEADERS}
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/bundle.pb.h
  COMMAND cp ${GENERATED_DIR}/signal.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.h ${GENERATED_INC_DIR}
  DEPENDS ${GENERATED_DIR}
)

add_custom_target(generate_protocpp ALL
  DEPENDS
  ${GENERATED_FILES}
  ${GENERATED_SRCS}
  ${GENERATED_HEADERS}
)

add_library(protoncpp
  src/proton.cpp
  src/config.cpp
  ${GENERATED_SRCS}
  ${GENERATED_HDRS}
)

target_include_directories(protoncpp
  PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
      ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(protoncpp PRIVATE protobuf::libprotobuf)
target_link_libraries(protoncpp PUBLIC yaml-cpp::yaml-cpp)

# Add tests
add_subdirectory(tests)