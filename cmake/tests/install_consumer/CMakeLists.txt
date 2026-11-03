cmake_minimum_required(VERSION 3.20)
project(proton_install_consumer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(TEST_NODE_BUILDER "Also compile the installed node-builder sources" OFF)

find_package(proton CONFIG REQUIRED)

set(generated_folder "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(generated_files
  "${generated_folder}/target_registry.c"
  "${generated_folder}/target_node.c"
  "${generated_folder}/target_registry_ids.h"
  "${generated_folder}/target_registry_sizes.h"
  "${generated_folder}/target_connections.h"
)
file(MAKE_DIRECTORY "${generated_folder}")
proton_core_generator(
  "${generated_files}"
  "${generated_folder}"
  producer
  CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/test.yaml"
)

add_executable(install_consumer
  main.cpp
  ${generated_files}
  ${PROTON_CORE_SOURCES}
  ${PROTON_CPP_SOURCES}
)

foreach(proton_target IN ITEMS proton::proton_core proton::proton_cpp)
  get_target_property(include_dirs ${proton_target} INTERFACE_INCLUDE_DIRECTORIES)
  if(include_dirs)
    target_include_directories(install_consumer PRIVATE ${include_dirs})
  endif()
  get_target_property(definitions ${proton_target} INTERFACE_COMPILE_DEFINITIONS)
  if(definitions)
    target_compile_definitions(install_consumer PRIVATE ${definitions})
  endif()
endforeach()
target_include_directories(install_consumer PRIVATE "${generated_folder}")

if(TEST_NODE_BUILDER)
  target_sources(install_consumer PRIVATE ${PROTON_CPP_NODE_BUILDER_SOURCES})
  target_link_libraries(install_consumer PRIVATE yaml-cpp nlohmann_json::nlohmann_json)
endif()

enable_testing()
add_test(NAME install_consumer COMMAND install_consumer)
