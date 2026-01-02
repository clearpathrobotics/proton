set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/j100.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(UTILS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

## PC

add_executable(j100_pc_cpp
  j100_pc.cpp
)

target_link_libraries(j100_pc_cpp
  PRIVATE
  proton::protoncpp
)

target_compile_definitions(j100_pc_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")
target_compile_options(j100_pc_cpp PRIVATE -Wno-narrowing)

# C Nodes

## MCU

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__j100_mcu.c"
  "${GENERATED_FOLDER}/proton__j100_mcu.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "mcu" True)

add_executable(j100_mcu_c
  j100_mcu.c
  ${GENERATED_FOLDER}/proton__j100_mcu.c
  ${UTILS_FOLDER}/utils.c
)

target_link_libraries(j100_mcu_c
  PRIVATE
  proton::protonc
)

target_include_directories(j100_mcu_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
        ${UTILS_FOLDER}
)

# Install
install(TARGETS j100_mcu_c j100_pc_cpp RUNTIME DESTINATION bin)