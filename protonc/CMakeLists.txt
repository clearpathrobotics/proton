cmake_minimum_required(VERSION 3.14)
project(proton C)

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
file(MAKE_DIRECTORY ${GENERATED_DIR})

# -------------------------------
# Custom command to run protoc + nanopb generator
# -------------------------------
set(GENERATED_SRCS)
set(GENERATED_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

    set(pb_c   ${GENERATED_DIR}/${PROTO_NAME}.pb.c)
    set(pb_h   ${GENERATED_DIR}/${PROTO_NAME}.pb.h)

    # Check if .options file exists
    set(OPTIONS_FILE ${PROTO_PATH}/${PROTO_NAME}.options)
    if(EXISTS ${OPTIONS_FILE})
        set(PROTO_DEPENDS ${PROTO_FILE} ${OPTIONS_FILE})
        message("Exists")
    else()
        set(PROTO_DEPENDS ${PROTO_FILE})
    endif()


    add_custom_command(
        OUTPUT ${pb_c} ${pb_h}
        COMMAND protoc
            --proto_path=${PROTO_PATH}
            --plugin=protoc-gen-nanopb=${nanopb_SOURCE_DIR}/generator/protoc-gen-nanopb
            --nanopb_out=${GENERATED_DIR}
            ${PROTO_FILE}
        DEPENDS ${PROTO_DEPENDS}
        WORKING_DIRECTORY ${PROTO_PATH}
        COMMENT "Generating nanopb files for ${PROTO_NAME}.proto"
        VERBATIM
    )

    list(APPEND GENERATED_SRCS ${pb_c})
    list(APPEND GENERATED_HDRS ${pb_h})
endforeach()

# Group generated files so IDEs show them
source_group("Generated" FILES ${GENERATED_SRCS} ${GENERATED_HDRS})

# -------------------------------
# proton library
# -------------------------------
add_library(proton
    Src/proton.c
    Src/proton_field_callbacks.c
    ${nanopb_SOURCE_DIR}/pb_common.c
    ${nanopb_SOURCE_DIR}/pb_decode.c
    ${nanopb_SOURCE_DIR}/pb_encode.c
    ${GENERATED_SRCS}
    ${GENERATED_HDRS}
)

target_include_directories(proton
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Inc
        ${nanopb_SOURCE_DIR}/
        ${GENERATED_DIR}                     # for generated .pb.h
)

# Add tests
add_subdirectory(tests)