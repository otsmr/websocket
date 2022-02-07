project(muster)
cmake_minimum_required(VERSION 2.8.12)

add_library(base64_lib ../websocket/base64/Base64.cpp ../websocket/base64/Base64.h)
add_library(sha1_lib ../websocket/sha1/sha1.cpp ../websocket/sha1/sha1.h)

add_executable(base64_test base64_test.cpp)
add_executable(sha1_test sha1_test.cpp)

target_link_libraries(base64_test PRIVATE base64_lib)
target_link_libraries(sha1_test PRIVATE sha1_lib)

enable_testing()

add_test(base64_test base64_test 0)
set_tests_properties(base64_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")

add_test(sha1_test sha1_test 0)
set_tests_properties(sha1_test PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED")