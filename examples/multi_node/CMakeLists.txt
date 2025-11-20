set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/multi_node.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(UTILS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

## MCU

add_executable(multi_node_1_cpp
  multi_node_1.cpp
)

target_link_libraries(multi_node_1_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(multi_node_1_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(multi_node_1_cpp PRIVATE -Wno-narrowing)

## ROS2

add_executable(multi_node_2_cpp
  multi_node_2.cpp
)

target_link_libraries(multi_node_2_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(multi_node_2_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(multi_node_2_cpp PRIVATE -Wno-narrowing)

## ros2_contorl

add_executable(multi_node_3_cpp
  multi_node_3.cpp
)

target_link_libraries(multi_node_3_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(multi_node_3_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(multi_node_3_cpp PRIVATE -Wno-narrowing)

# C Nodes

## MCU

# set(GENERATED_C_FILES
#   "${GENERATED_FOLDER}/proton__multi_node_mcu.c"
#   "${GENERATED_FOLDER}/proton__multi_node_mcu.h"
# )

# protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "mcu")

# add_executable(multi_node_mcu_c
#   multi_node_mcu.c
#   ${GENERATED_FOLDER}/proton__multi_node_mcu.c
#   ${UTILS_FOLDER}/utils.c
# )

# target_link_libraries(multi_node_mcu_c
#   PRIVATE
#   proton::protonc
# )

# target_include_directories(multi_node_mcu_c
#     PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#         ${CMAKE_SOURCE_DIR}/nanopb
#         ${GENERATED_FOLDER}
#         ${UTILS_FOLDER}
# )

# ## ROS2

# set(GENERATED_C_FILES
#   "${GENERATED_FOLDER}/proton__multi_node_ros2.c"
#   "${GENERATED_FOLDER}/proton__multi_node_ros2.h"
# )

# protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "ros2")

# add_executable(multi_node_ros2_c
#   multi_node_ros2.c
#   ${GENERATED_FOLDER}/proton__multi_node_ros2.c
#   ${UTILS_FOLDER}/utils.c
# )

# target_link_libraries(multi_node_ros2_c
#   PRIVATE
#   proton::protonc
# )

# target_include_directories(multi_node_ros2_c
#     PRIVATE
#         ${CMAKE_SOURCE_DIR}/include
#         ${CMAKE_SOURCE_DIR}/nanopb
#         ${GENERATED_FOLDER}
#         ${UTILS_FOLDER}
# )

# Install
install(TARGETS
  #multi_node_mcu_c
  multi_node_1_cpp
  #multi_node_ros2_c
  multi_node_2_cpp
  multi_node_3_cpp
RUNTIME DESTINATION bin)
