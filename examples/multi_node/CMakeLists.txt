set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/multi_node.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(UTILS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

## Node 1

add_executable(multi_node_1_cpp
  multi_node_1.cpp
)

target_link_libraries(multi_node_1_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(multi_node_1_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(multi_node_1_cpp PRIVATE -Wno-narrowing)

## Node 2

add_executable(multi_node_2_cpp
  multi_node_2.cpp
)

target_link_libraries(multi_node_2_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(multi_node_2_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(multi_node_2_cpp PRIVATE -Wno-narrowing)

## Node 3

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

## Node 1

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__multi_node_node1.c"
  "${GENERATED_FOLDER}/proton__multi_node_node1.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "node1" True)

add_executable(multi_node_1_c
  multi_node_1.c
  ${GENERATED_FOLDER}/proton__multi_node_node1.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(multi_node_1_c
  PRIVATE
  proton::protonc
)

target_include_directories(multi_node_1_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

## Node 2

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__multi_node_node2.c"
  "${GENERATED_FOLDER}/proton__multi_node_node2.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "node2" True)

add_executable(multi_node_2_c
  multi_node_2.c
  ${GENERATED_FOLDER}/proton__multi_node_node2.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(multi_node_2_c
  PRIVATE
  proton::protonc
)

target_include_directories(multi_node_2_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

## Node 3

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__multi_node_node3.c"
  "${GENERATED_FOLDER}/proton__multi_node_node3.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "node3" True)

add_executable(multi_node_3_c
  multi_node_3.c
  ${GENERATED_FOLDER}/proton__multi_node_node3.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(multi_node_3_c
  PRIVATE
  proton::protonc
)

target_include_directories(multi_node_3_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

# Install
install(TARGETS
  multi_node_1_c
  multi_node_2_c
  multi_node_3_c
  multi_node_1_cpp
  multi_node_2_cpp
  multi_node_3_cpp
RUNTIME DESTINATION bin)
