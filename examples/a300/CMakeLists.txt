set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/a300.yaml)
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

# C++ Nodes

add_executable(a300_pc_cpp
  a300_pc.cpp
)

target_link_libraries(a300_pc_cpp
  protoncpp
)

target_compile_definitions(a300_pc_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__a300_mcu.c"
  "${GENERATED_FOLDER}/proton__a300_mcu.h"
)

add_custom_command(
  OUTPUT ${GENERATED_C_FILES}
  COMMAND ${PROTONC_GENERATOR}
    -c ${CONFIG_FILE}
    -d ${GENERATED_FOLDER}
    -t "mcu"
  DEPENDS ${CONFIG_FILE}
  COMMENT "Generating Proton C files for A300"
)

add_executable(a300_mcu_c
  a300_mcu.c
  ${GENERATED_FOLDER}/proton__a300_mcu.c
)

target_link_libraries(a300_mcu_c
    protonc
)

target_include_directories(a300_mcu_c
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
)
