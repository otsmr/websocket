
project(
    from-scratch-tests
    LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)

include_directories("../src/")

enable_testing()

# TEST Base64 
add_executable(
    base64_test base64_test.cpp
    ../src/base64/base64.cpp
)
target_include_directories(base64_test PRIVATE "../src")
add_test(base64_test base64_test 0)
set_tests_properties(base64_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

# TEST sha1
add_executable(
    sha1_test sha1_test.cpp
    ../src/sha1/sha1.cpp
)
target_include_directories(sha1_test PRIVATE "../src")
add_test(sha1_test sha1_test 0)
set_tests_properties(sha1_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

# TEST sha256
add_executable(
    sha256_test sha256_test.cpp
    ../src/sha256/sha256.cpp
)
target_include_directories(sha256_test PRIVATE "../src")
add_test(sha256_test sha256_test 0)
set_tests_properties(sha256_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

# TEST websocket frames
add_executable(
    dataframe_test dataframe_test.cpp
    ../src/websocket/frame.cpp
)
target_include_directories(dataframe_test PRIVATE "../src")
add_test(dataframe_test dataframe_test 0)
set_tests_properties(dataframe_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")