file(GLOB TEST_SOURCES *.c)
set(GENERATED_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/generated)

foreach(test_file ${TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file} ${GENERATED_FOLDER}/proton__sample.c)

    target_link_libraries(${test_name}
        proton
    )

    target_include_directories(${test_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/Inc
            ${CMAKE_SOURCE_DIR}/nanopb
            ${GENERATED_FOLDER}/
    )
endforeach()
