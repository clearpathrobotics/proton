cmake_minimum_required(VERSION 3.14)
project(protoncpp CXX)


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

    set(pb_cpp   ${GENERATED_DIR}/${PROTO_NAME}.pb.cc)
    set(pb_hpp   ${GENERATED_DIR}/${PROTO_NAME}.pb.hh)

    add_custom_command(
        OUTPUT ${pb_cpp} ${pb_hpp}
        COMMAND protoc
            --proto_path=${PROTO_PATH}
            --cpp_out=${GENERATED_DIR}
            ${PROTO_FILE}
        DEPENDS  ${PROTO_FILE}
        COMMENT "Generating cpp protobuf files for ${PROTO_NAME}.proto"
    )

    list(APPEND GENERATED_SRCS ${pb_cpp})
    list(APPEND GENERATED_HDRS ${pb_hpp})
endforeach()

add_library(protoncpp
  Src/proton.cpp
  ${GENERATED_SRCS}
  ${GENERATED_HDRS}
)

target_include_directories(protoncpp
  PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/Inc
      ${GENERATED_DIR}
)

# Add tests
#add_subdirectory(tests)