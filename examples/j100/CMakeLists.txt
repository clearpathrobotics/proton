set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/j100.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

## MCU

add_executable(j100_mcu_cpp
  j100_mcu.cpp
)

target_link_libraries(j100_mcu_cpp
  protoncpp
)

target_compile_definitions(j100_mcu_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")

## PC

add_executable(j100_pc_cpp
  j100_pc.cpp
)

target_link_libraries(j100_pc_cpp
  protoncpp
)

target_compile_definitions(j100_pc_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")

# C Nodes

## MCU

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__j100_mcu.c"
  "${GENERATED_FOLDER}/proton__j100_mcu.h"
)

add_custom_command(
  OUTPUT ${GENERATED_C_FILES}
  COMMAND ${PROTONC_GENERATOR}
    -c ${CONFIG_FILE}
    -d ${GENERATED_FOLDER}
    -t "mcu"
  DEPENDS ${CONFIG_FILE}
  COMMENT "Generating Proton C files for j100"
)

add_executable(j100_mcu_c
  j100_mcu.c
  ${GENERATED_FOLDER}/proton__j100_mcu.c
)

target_link_libraries(j100_mcu_c
    protonc
)

target_include_directories(j100_mcu_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
)

## PC

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__j100_pc.c"
  "${GENERATED_FOLDER}/proton__j100_pc.h"
)

add_custom_command(
  OUTPUT ${GENERATED_C_FILES}
  COMMAND ${PROTONC_GENERATOR}
    -c ${CONFIG_FILE}
    -d ${GENERATED_FOLDER}
    -t "pc"
  DEPENDS ${CONFIG_FILE}
  COMMENT "Generating Proton C files for j100"
)

add_executable(j100_pc_c
  j100_pc.c
  ${GENERATED_FOLDER}/proton__j100_pc.c
)

target_link_libraries(j100_pc_c
    protonc
)

target_include_directories(j100_pc_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
)
