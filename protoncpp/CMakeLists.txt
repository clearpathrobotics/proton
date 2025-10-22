cmake_minimum_required(VERSION 3.14)
project(protoncpp CXX)

# Protobuf compiler
# find_package(Protobuf REQUIRED)
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()

# YAML-CPP
find_package(yaml-cpp REQUIRED)

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
      --cpp_out=${GENERATED_DIR}
      ${PROTO_FILES}
  DEPENDS ${PROTO_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating cpp protobuf files for ${PROTO_NAME}.proto"
)

add_custom_command(
  OUTPUT ${GENERATED_SRCS}
  COMMAND sed -i 's|"bundle.pb.h"|"${PROJECT_NAME}/bundle.pb.h"|' ${GENERATED_DIR}/bundle.pb.cc
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/signal.pb.cc
  COMMAND cp ${GENERATED_DIR}/signal.pb.cc ${GENERATED_SRC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.cc ${GENERATED_SRC_DIR}
  DEPENDS ${PROTO_FILES}
)

add_custom_command(
  OUTPUT ${GENERATED_HEADERS}
  COMMAND sed -i 's|"signal.pb.h"|"${PROJECT_NAME}/signal.pb.h"|' ${GENERATED_DIR}/bundle.pb.h
  COMMAND cp ${GENERATED_DIR}/signal.pb.h ${GENERATED_INC_DIR}
  COMMAND cp ${GENERATED_DIR}/bundle.pb.h ${GENERATED_INC_DIR}
  DEPENDS ${PROTO_FILES}
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
  src/signal.cpp
  src/bundle.cpp
  src/bundle_manager.cpp
  src/transport/udp4.cpp
  src/transport/serial.cpp
  ${GENERATED_SRCS}
  ${GENERATED_HDRS}
)

add_library(proton::protoncpp ALIAS protoncpp)

target_include_directories(protoncpp
  PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(protoncpp PRIVATE protobuf::libprotobuf)
target_link_libraries(protoncpp PUBLIC yaml-cpp)

# Install

install(TARGETS protoncpp
    EXPORT ProtonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${GENERATED_INC_DIR}/ DESTINATION include/protoncpp)
