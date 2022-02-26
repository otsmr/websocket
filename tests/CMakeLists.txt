
project(
    from-scratch-tests
    LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)

include_directories("../src/")

add_executable(
    base64_test
    base64_test.cpp
    
    ../src/base64/base64.cpp
)
target_include_directories(base64_test PRIVATE "../src")

add_executable(
    sha1_test
    sha1_test.cpp

    ../src/sha1/sha1.cpp
)
target_include_directories(sha1_test PRIVATE "../src")

add_executable(
    dataframe_test
    dataframe_test.cpp

    ../src/websocket/frame.cpp
)
target_include_directories(dataframe_test PRIVATE "../src")



enable_testing()

add_test(base64_test base64_test 0)
set_tests_properties(base64_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

add_test(sha1_test sha1_test 0)
set_tests_properties(sha1_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

add_test(dataframe_test dataframe_test 0)
set_tests_properties(dataframe_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")