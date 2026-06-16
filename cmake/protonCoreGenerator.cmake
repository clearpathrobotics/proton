function(proton_core_generator GENERATED_FILES GENERATED_FOLDER TARGET)
  # CONFIG_FILE is optional - use PROTON_CORE_CONFIG_FILE if set by user
  set(options "")
  set(oneValueArgs CONFIG_FILE)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_CONFIG_FILE)
    set(CONFIG_FILE ${ARG_CONFIG_FILE})
  elseif(PROTON_CORE_CONFIG_FILE)
    set(CONFIG_FILE ${PROTON_CORE_CONFIG_FILE})
  endif()

  # Build optional config argument
  set(CONFIG_ARG "")
  set(CONFIG_DEPENDS "")
  if(CONFIG_FILE)
    set(CONFIG_ARG -c ${CONFIG_FILE})
    if(EXISTS ${CONFIG_FILE})
      set(CONFIG_DEPENDS ${CONFIG_FILE})
    endif()
  endif()

  get_filename_component(PROTON_CORE_CMAKE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY)
  get_filename_component(PROTON_ROOT_DIR "${PROTON_CORE_CMAKE_DIR}" DIRECTORY)
  set(PROTON_CORE_GENERATOR_SCRIPT "${PROTON_ROOT_DIR}/generator_scripts/generator.py")
  set(PROTON_CORE_PYTHONPATH "$ENV{PYTHONPATH}:${PROTON_ROOT_DIR}/generator_scripts")


  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  # Add a custom target to execute the script
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${CMAKE_COMMAND} -E env
      PYTHONPATH=${PROTON_CORE_PYTHONPATH}
      ${Python3_EXECUTABLE}
      ${PROTON_CORE_GENERATOR_SCRIPT}
      ${CONFIG_ARG}
      -d ${GENERATED_FOLDER}
      -t ${TARGET}
    DEPENDS ${CONFIG_DEPENDS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Python script: ${Python3_EXECUTABLE} ${PROTON_CORE_GENERATOR} ${CONFIG_ARG} -d ${GENERATED_FOLDER} -t ${TARGET}"
  )
endfunction()
