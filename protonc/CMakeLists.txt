cmake_minimum_required(VERSION 3.14)
project(Proton C)

# Output directory for generated nanopb files
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(GENERATED_PROTO_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

# All .proto files
file(GLOB PROTO_FILES ${PROTO_DIR}/*.proto)

# Path to the nanopb generator plugin
# Update this to match where yours is located, or use pip-installed one
set(NANOPB_PLUGIN_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/nanopb/generator/protoc-gen-nanopb" CACHE FILEPATH "Path to protoc-gen-nanopb")

# Generated source files
set(GENERATED_SOURCES "")

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(OUT_H ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.h)
    set(OUT_C ${GENERATED_PROTO_DIR}/${PROTO_NAME}.pb.c)

    # Check if .options file exists
    set(OPTIONS_FILE ${PROTO_DIR}/${PROTO_NAME}.options)
    if(EXISTS ${OPTIONS_FILE})
        set(PROTO_DEPENDS ${PROTO_FILE} ${OPTIONS_FILE})
    else()
        set(PROTO_DEPENDS ${PROTO_FILE})
    endif()

    add_custom_command(
      OUTPUT ${OUT_H} ${OUT_C}
      COMMAND protoc
          --plugin=protoc-gen-nanopb=${NANOPB_PLUGIN_EXECUTABLE}
          --nanopb_out=${GENERATED_PROTO_DIR}
          -I ${PROTO_DIR}
          ${PROTO_NAME}.proto
      DEPENDS ${PROTO_DEPENDS}
      WORKING_DIRECTORY ${PROTO_DIR}
      COMMENT "Generating ${PROTO_NAME}.pb.[ch] with nanopb and options"
    )

    list(APPEND GENERATED_SOURCES ${OUT_H} ${OUT_C})
endforeach()

add_custom_target(generate_protos ALL DEPENDS ${GENERATED_SOURCES})

# Create the Proton library
add_library(proton
    Src/proton.c
    Src/proton_field_callbacks.c
    nanopb/pb_common.c
    nanopb/pb_decode.c
    nanopb/pb_encode.c
    ${GENERATED_SOURCES}
)
add_dependencies(proton generate_protos)

# Include directories for Proton library
target_include_directories(proton
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/nanopb
        ${GENERATED_PROTO_DIR}
)

# Set C standard (optional)
set_target_properties(proton PROPERTIES C_STANDARD 99)

# Add tests
add_subdirectory(tests)