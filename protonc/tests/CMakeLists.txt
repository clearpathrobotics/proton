file(GLOB TEST_SOURCES *.c)

# foreach(test_file ${TEST_SOURCES})
#     get_filename_component(test_name ${test_file} NAME_WE)
#     add_executable(${test_name} ${test_file})

#     target_link_libraries(${test_name}
#         proton
#     )

#     target_include_directories(${test_name}
#         PRIVATE
#             ${CMAKE_SOURCE_DIR}/Inc
#             ${CMAKE_SOURCE_DIR}/nanopb
#     )
# endforeach()

#add_subdirectory(sample)
add_subdirectory(a300)