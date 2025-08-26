set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/config/a300.yaml)

add_executable(a300_cpp
  a300.cpp
)

target_link_libraries(a300_cpp
    protoncpp
)

# target_include_directories(a300
#     PRIVATE
#         ${CMAKE_SOURCE_DIR}/Inc
#         ${CMAKE_SOURCE_DIR}/nanopb
#         ${GENERATED_FOLDER}
# )
