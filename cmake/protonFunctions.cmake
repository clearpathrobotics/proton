
function(protonc_generator GENERATED_FILES CONFIG_FILE GENERATED_FOLDER TARGET GENERATE_NODE)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  # Get the directory where this function is defined (proton/cmake)
  get_filename_component(PROTON_CMAKE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY)

  # When installed to lib/cmake/proton, the prefix is 4 levels up:
  # <prefix>/lib/cmake/proton/protonFunctions.cmake
  # We need to go to <prefix>/share/proton/protonc
  get_filename_component(_prefix "${PROTON_CMAKE_DIR}" DIRECTORY)  # lib/cmake
  get_filename_component(_prefix "${_prefix}" DIRECTORY)            # lib
  get_filename_component(_prefix "${_prefix}" DIRECTORY)            # <prefix>

  set(PROTONC_GENERATOR_SCRIPT "${_prefix}/share/proton/protonc/protonc_generator.py")
  set(PROTONC_PYTHONPATH "${_prefix}/share/proton")

  # Add a custom command to execute the script
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E env
      PYTHONPATH=${PROTONC_PYTHONPATH}
      ${Python3_EXECUTABLE}
      ${PROTONC_GENERATOR_SCRIPT}
      -c ${CONFIG_FILE}
      -d ${GENERATED_FOLDER}
      -t ${TARGET}
      -n ${GENERATE_NODE}
    DEPENDS ${CONFIG_FILE} # Regenerate if config changes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python script: ${Python3_EXECUTABLE} ${PROTONC_GENERATOR_SCRIPT} -c ${CONFIG_FILE} -d ${GENERATED_FOLDER} -t ${TARGET} -n ${GENERATE_NODE}"
  )
endfunction()