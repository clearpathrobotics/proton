set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/a300.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(UTILS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

## MCU

add_executable(a300_mcu_cpp
  a300_mcu.cpp
)

target_link_libraries(a300_mcu_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(a300_mcu_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(a300_mcu_cpp PRIVATE -Wno-narrowing)

## PC

add_executable(a300_pc_cpp
  a300_pc.cpp
)

target_link_libraries(a300_pc_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(a300_pc_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(a300_pc_cpp PRIVATE -Wno-narrowing)

# C Nodes

## MCU

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__a300_mcu.c"
  "${GENERATED_FOLDER}/proton__a300_mcu.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "mcu")

add_executable(a300_mcu_c
  a300_mcu.c
  ${GENERATED_FOLDER}/proton__a300_mcu.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(a300_mcu_c
  PRIVATE
  proton::protonc
)

target_include_directories(a300_mcu_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

## PC

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__a300_pc.c"
  "${GENERATED_FOLDER}/proton__a300_pc.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "pc")

add_executable(a300_pc_c
  a300_pc.c
  ${GENERATED_FOLDER}/proton__a300_pc.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(a300_pc_c
  PRIVATE
  proton::protonc
)

target_include_directories(a300_pc_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

# Install
install(TARGETS a300_mcu_c a300_mcu_cpp a300_pc_c a300_pc_cpp RUNTIME DESTINATION bin)
