
function(protonc_generator GENERATED_FILES CONFIG_FILE GENERATED_FOLDER TARGET)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  # Add a custom target to execute the script
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E env
      PYTHONPATH=${CMAKE_SOURCE_DIR}/protonc
      ${Python3_EXECUTABLE}
      ${PROTONC_GENERATOR}
      -c ${CONFIG_FILE}
      -d ${GENERATED_FOLDER}
      -t ${TARGET}
    DEPENDS ${CONFIG_FILE} # Regenerate if config changes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python script: ${Python3_EXECUTABLE} ${PROTONC_GENERATOR} -c ${CONFIG_FILE} -d ${GENERATED_FOLDER} -t ${TARGET}"
  )
endfunction()