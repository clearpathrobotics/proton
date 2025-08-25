
set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__a300_mcu.c"
  "${GENERATED_FOLDER}/proton__a300_mcu.h"
)
set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/config/a300.yaml)

file(MAKE_DIRECTORY "${GENERATED_FOLDER}")

add_custom_command(
  OUTPUT ${GENERATED_C_FILES}
  COMMAND ${PROTONC_GENERATOR}
    -c ${CONFIG_FILE}
    -d ${GENERATED_FOLDER}
    -t "mcu"
  DEPENDS ${CONFIG_FILE}
  COMMENT "Generating Proton C files for A300"
)

add_executable(a300
  a300.c
  ${GENERATED_FOLDER}/proton__a300_mcu.c
)

target_link_libraries(a300
    proton
)

target_include_directories(a300
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Inc
        ${CMAKE_SOURCE_DIR}/nanopb
        ${GENERATED_FOLDER}
)
