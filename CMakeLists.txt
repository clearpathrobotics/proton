cmake_minimum_required(VERSION 3.8)
project(proton)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#find_package(protobuf-compiler REQUIRED)

add_subdirectory(protonc)
add_subdirectory(protoncpp)
add_subdirectory(examples)