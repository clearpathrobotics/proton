cmake_minimum_required(VERSION 3.8)
project(proton)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(clearpath_platform_msgs REQUIRED)

# find_package(python3-protobuf REQUIRED)

# Protobuf compiler
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
  message(FATAL_ERROR "Could not find protoc")
endif()


file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")

set(GENERATED_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated_cpp")
set(GENERATED_PY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proton/proton_py")

file(MAKE_DIRECTORY "${GENERATED_CPP_DIR}")
file(MAKE_DIRECTORY "${GENERATED_PY_DIR}")

# Compile .proto to .pb.h/.pb.cc (C++)
# foreach(PROTO_FILE ${PROTO_FILES})
#   get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

#   add_custom_command(
#     OUTPUT "${GENERATED_CPP_DIR}/${PROTO_NAME}.pb.cc"
#            "${GENERATED_CPP_DIR}/${PROTO_NAME}.pb.h"
#     COMMAND ${PROTOC_EXECUTABLE}
#       --cpp_out=${GENERATED_CPP_DIR}
#       -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
#       ${PROTO_FILE}
#     DEPENDS ${PROTO_FILE}
#     COMMENT "Generating C++ from ${PROTO_FILE}"
#   )

#   list(APPEND GENERATED_CPP_SRCS "${GENERATED_CPP_DIR}/${PROTO_NAME}.pb.cc")
#   list(APPEND GENERATED_CPP_HDRS "${GENERATED_CPP_DIR}/${PROTO_NAME}.pb.h")
# endforeach()

# add_library(proto_msgs ${GENERATED_CPP_SRCS})
# target_include_directories(proto_msgs PUBLIC ${GENERATED_CPP_DIR})
# ament_target_dependencies(proto_msgs protobuf)

# Install C++ files
# install(DIRECTORY ${GENERATED_CPP_DIR}/
#   DESTINATION include/${PROJECT_NAME}
#   FILES_MATCHING PATTERN "*.h"
# )

# install(TARGETS proto_msgs
#   EXPORT export_${PROJECT_NAME}
#   ARCHIVE DESTINATION lib
#   LIBRARY DESTINATION lib
#   RUNTIME DESTINATION bin
# )


# Python .proto generation
foreach(PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

  set(GENERATED_PY_FILE "${GENERATED_PY_DIR}/${PROTO_NAME}_pb2.py")
  set(GENERATED_PYI_FILE "${GENERATED_PY_DIR}/${PROTO_NAME}_pb2.pyi")

  add_custom_command(
    OUTPUT ${GENERATED_PY_FILE}
    COMMAND ${PROTOC_EXECUTABLE}
      --python_out=${GENERATED_PY_DIR}
      -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
      ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating Python proto: ${PROTO_FILE}"
  )

  add_custom_command(
    OUTPUT ${GENERATED_PYI_FILE}
    COMMAND ${PROTOC_EXECUTABLE}
      --pyi_out=${GENERATED_PY_DIR}
      -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
      ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating Python proto: ${PROTO_FILE}"
  )

  list(APPEND GENERATED_PY_FILES ${GENERATED_PY_FILE})
  list(APPEND GENERATED_PYI_FILES ${GENERATED_PYI_FILE})
endforeach()

add_custom_target(generate_proto_py ALL DEPENDS ${GENERATED_PY_FILES})
add_custom_target(generate_proto_pyi ALL DEPENDS ${GENERATED_PYI_FILES})

install(
    PROGRAMS ${PROJECT_NAME}/a300_ros2_bridge
    DESTINATION lib/${PROJECT_NAME}
  )

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_python_install_package(${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
