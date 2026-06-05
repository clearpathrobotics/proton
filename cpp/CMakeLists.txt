cmake_minimum_required(VERSION 3.14)
project(proton_core_cpp C CXX)

set(PROTON_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Add proton_core dependency if not already defined
if(NOT TARGET proton_core)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../core ${CMAKE_CURRENT_BINARY_DIR}/core)
endif()

add_library(${PROJECT_NAME}
  src/bundle_access.cpp
  src/signal_access.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROTON_COMMON_INCLUDE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    proton::proton_core
)

add_library(proton::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
