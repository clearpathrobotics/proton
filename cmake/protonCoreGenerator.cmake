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

  # Locate generator.py — try the install-tree layout first
  # (<prefix>/lib/cmake/proton/generator_scripts/generator.py), then fall back
  # to the source-tree layout (<proton-src>/generator_scripts/generator.py).
  get_filename_component(_proton_cmake_dir
    "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" DIRECTORY)

  set(_candidates
    "${_proton_cmake_dir}/generator_scripts/generator.py"
    "${_proton_cmake_dir}/../generator_scripts/generator.py"
  )

  set(PROTON_CORE_GENERATOR_SCRIPT "")
  foreach(_c IN LISTS _candidates)
    if(EXISTS "${_c}")
      get_filename_component(PROTON_CORE_GENERATOR_SCRIPT "${_c}" ABSOLUTE)
      break()
    endif()
  endforeach()

  if(NOT PROTON_CORE_GENERATOR_SCRIPT)
    message(FATAL_ERROR
      "proton_core_generator: generator.py not found near ${_proton_cmake_dir}")
  endif()

  get_filename_component(_proton_generator_dir
    "${PROTON_CORE_GENERATOR_SCRIPT}" DIRECTORY)
  set(PROTON_CORE_PYTHONPATH "$ENV{PYTHONPATH}:${_proton_generator_dir}")

  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  # Add a custom command to execute the script
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
    COMMENT "Running proton generator: ${PROTON_CORE_GENERATOR_SCRIPT} ${CONFIG_ARG} -d ${GENERATED_FOLDER} -t ${TARGET}"
  )
endfunction()
