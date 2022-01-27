
# version of CMake
cmake_minimum_required(VERSION 3.11)

# infos about the project
project(
  from-scratch
  VERSION 0.1
  DESCRIPTION "WebSocket from scratch"
  LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "./build")

add_library(base64 base64/base64.cpp base64/base64.h)
add_library(sha1 sha1/sha1.cpp sha1/sha1.h)

add_executable(server server.cpp)

target_link_libraries(server PRIVATE base64)
target_link_libraries(server PRIVATE sha1)