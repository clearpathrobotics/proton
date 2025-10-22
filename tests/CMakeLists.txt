# Testing

# GTest
find_package(GTest REQUIRED)

set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test.yaml)

## Protonc

set(GENERATED_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/generated)

## Signals
set(GENERATED_C_FILES
  "${GENERATED_FOLDER}/proton__test_producer.c"
  "${GENERATED_FOLDER}/proton__test_producer.h"
)

protonc_generator("${GENERATED_C_FILES}" ${CONFIG_FILE} ${GENERATED_FOLDER} "producer")

add_executable(signal_test_c
  protonc/signal_test.cpp
  protonc/utils.cpp
  ${GENERATED_FOLDER}/proton__test_producer.c
)

target_link_libraries(signal_test_c PRIVATE
  GTest::gtest_main
  proton::protonc
)

target_include_directories(signal_test_c PRIVATE
  ${GENERATED_FOLDER}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

## Protoncpp

## Signals
add_executable(signal_test_cpp protoncpp/signal_test.cpp protoncpp/utils.cpp)
target_include_directories(signal_test_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(signal_test_cpp GTest::gtest_main proton::protoncpp)
target_compile_definitions(signal_test_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")

## Bundles
add_executable(bundle_test_cpp protoncpp/bundle_test.cpp protoncpp/utils.cpp)
target_include_directories(bundle_test_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bundle_test_cpp GTest::gtest_main proton::protoncpp)
target_compile_definitions(bundle_test_cpp PRIVATE CONFIG_FILE="${CONFIG_FILE}")

include(GoogleTest)
gtest_discover_tests(signal_test_c)
gtest_discover_tests(signal_test_cpp)
gtest_discover_tests(bundle_test_cpp)