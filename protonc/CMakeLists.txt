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
  SOURCE_SUBDIR  null
)
# FetchContent_MakeAvailable(nanopb)

FetchContent_GetProperties(nanopb)
if(NOT proton_POPULATED)
  FetchContent_Populate(nanopb)
endif()


# Protobuf compiler
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()

# Protonc generator
function(protonc_generator GENERATED_FILES CONFIG_FILE GENERATED_FOLDER TARGET)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(PROTONC_GENERATOR ${CMAKE_SOURCE_DIR}/protonc/protonc/protonc_generator.py)
  # Add a custom target to execute the script
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E env
      PYTHONPATH=${CMAKE_SOURCE_DIR}/protonc
      ${Python3_EXECUTABLE}
      ${PROTONC_GENERATOR}
      -c ${CONFIG_FILE}
      -d ${GENERATED_FOLDER}
      -t ${TARGET}
    DEPENDS ${CONFIG_FILE} # Regenerate if config changes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python script: ${Python3_EXECUTABLE} ${PROTONC_GENERATOR} -c ${CONFIG_FILE} -d ${GENERATED_FOLDER} -t ${TARGET}"
  )
endfunction()

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
  DEPENDS ${GENERATED_FILES}
)

add_custom_command(
  OUTPUT ${GENERATED_HEADERS}
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/bundle.pb.h
  COMMAND cp ${GENERATED_DIR}/signal.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.h ${GENERATED_INC_DIR}
  DEPENDS ${GENERATED_FILES}
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
add_library(protonc
    src/proton.c
    src/proton_field_callbacks.c
    ${nanopb_SOURCE_DIR}/pb_common.c
    ${nanopb_SOURCE_DIR}/pb_decode.c
    ${nanopb_SOURCE_DIR}/pb_encode.c
    ${GENERATED_SRCS}
    ${GENERATED_HEADERS}
)

add_library(proton::protonc ALIAS protonc)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${nanopb_SOURCE_DIR}>
)

# Install

install(TARGETS protonc
    EXPORT ProtonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY
        include/
        DESTINATION include)

install(DIRECTORY
        ${GENERATED_INC_DIR}/
        DESTINATION include/protonc)

