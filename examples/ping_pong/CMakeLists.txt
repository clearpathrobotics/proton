cmake_minimum_required(VERSION 3.14)
project(proton_examples_ping_pong C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROTON_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
get_filename_component(PING_PONG_CONFIG ${CMAKE_CURRENT_LIST_DIR}/config/ping_pong.yaml ABSOLUTE)

# PROTON_NODE_BUILDER requires PROTON_ENABLE_ALLOC.
set(PROTON_ENABLE_ALLOC ON CACHE BOOL "" FORCE)
set(PROTON_ENABLE_LOGGING ON CACHE BOOL "" FORCE)
set(PROTON_NODE_BUILDER ON CACHE BOOL "" FORCE)
set(PROTON_NODE_BUILDER_YAML_PARSER ON CACHE BOOL "" FORCE)

if(NOT TARGET proton::proton_cpp)
  add_subdirectory(${PROTON_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/proton)
endif()

set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(GENERATED_REGISTRY_FILES
  "${GENERATED_FOLDER}/target_registry.c"
  "${GENERATED_FOLDER}/target_node.c"
  "${GENERATED_FOLDER}/target_registry_ids.h"
  "${GENERATED_FOLDER}/target_registry_sizes.h"
  "${GENERATED_FOLDER}/target_connections.h"
)

include(${PROTON_ROOT}/cmake/protonCoreGenerator.cmake)
proton_core_generator(
  "${GENERATED_REGISTRY_FILES}"
  ${GENERATED_FOLDER}
  "pinger"
  CONFIG_FILE ${PING_PONG_CONFIG}
)

add_executable(${PROJECT_NAME} src/main.cpp ${GENERATED_REGISTRY_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${GENERATED_FOLDER})

# The ponger builds its node from this config at runtime, so it needs the path baked in
target_compile_definitions(${PROJECT_NAME} PRIVATE PING_PONG_CONFIG_PATH="${PING_PONG_CONFIG}")

target_link_libraries(${PROJECT_NAME} PRIVATE proton::proton_cpp)
