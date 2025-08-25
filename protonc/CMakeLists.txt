cmake_minimum_required(VERSION 3.14)
project(protonc C)

include(FetchContent)
# -------------------------------
# Fetch nanopb (runtime + generator)
# -------------------------------
set(nanopb_BUILD_GENERATOR ON CACHE BOOL "Build nanopb generator" FORCE)
FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG        0.4.9.1
)
FetchContent_MakeAvailable(nanopb)

# Protobuf compiler
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()

# -------------------------------
# Proto files
# -------------------------------
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(PROTO_FILES
    ${PROTO_PATH}/signal.proto
    ${PROTO_PATH}/bundle.proto
)

set(PROTO_OPTION_FILES
    ${PROTO_PATH}/signal.options
    ${PROTO_PATH}/bundle.options
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
set(GENERATED_FILES
  "${GENERATED_DIR}/signal.pb.c"
  "${GENERATED_DIR}/signal.pb.h"
  "${GENERATED_DIR}/bundle.pb.c"
  "${GENERATED_DIR}/bundle.pb.h"
)

set(GENERATED_SRCS
  "${GENERATED_SRC_DIR}/signal.pb.c"
  "${GENERATED_SRC_DIR}/bundle.pb.c"
)

set(GENERATED_HEADERS
  "${GENERATED_INC_DIR}/signal.pb.h"
  "${GENERATED_INC_DIR}/bundle.pb.h"
)

add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND protoc
        --proto_path=${PROTO_PATH}
        --plugin=protoc-gen-nanopb=${nanopb_SOURCE_DIR}/generator/protoc-gen-nanopb
        --nanopb_out=${GENERATED_DIR} signal.proto bundle.proto
    DEPENDS ${PROTO_DEPENDS}
    WORKING_DIRECTORY ${PROTO_PATH}
    COMMENT "Generating nanopb files for ${PROTO_NAME}.proto"
)

add_custom_command(
  OUTPUT ${GENERATED_SRCS}
  COMMAND sed -i 's|"bundle.pb.h"|"${PROJECT_NAME}/bundle.pb.h"|' ${GENERATED_DIR}/bundle.pb.c
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/signal.pb.c
  COMMAND cp ${GENERATED_DIR}/signal.pb.c ${GENERATED_SRC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.c ${GENERATED_SRC_DIR}
  DEPENDS ${GENERATED_DIR}
)

add_custom_command(
  OUTPUT ${GENERATED_HEADERS}
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/bundle.pb.h
  COMMAND cp ${GENERATED_DIR}/signal.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.h ${GENERATED_INC_DIR}
  DEPENDS ${GENERATED_DIR}
)

add_custom_target(generate_protoc ALL
  DEPENDS
  ${GENERATED_FILES}
  ${GENERATED_SRCS}
  ${GENERATED_HEADERS}
)

# -------------------------------
# protonc library
# -------------------------------
add_library(${PROJECT_NAME}
    src/proton.c
    src/proton_field_callbacks.c
    ${nanopb_SOURCE_DIR}/pb_common.c
    ${nanopb_SOURCE_DIR}/pb_decode.c
    ${nanopb_SOURCE_DIR}/pb_encode.c
    ${GENERATED_SRCS}
    ${GENERATED_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${nanopb_SOURCE_DIR}
)

# -------------------------------
# Protonc generator
# -------------------------------
find_program(PROTONC_GENERATOR protonc_generator)


# Add tests
add_subdirectory(tests)