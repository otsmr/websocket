
# version of CMake
cmake_minimum_required(VERSION 3.11)

# infos about the project
project(
  from-scratch
  VERSION 0.1
  DESCRIPTION "WebSocket from scratch"
  LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "./build")
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -c")

add_library(base64 base64/base64.cpp base64/base64.h)
add_library(sha1 sha1/sha1.cpp sha1/sha1.h)

add_executable(main
  server.cpp
  main.cpp
  )

target_link_libraries(main PRIVATE base64)
target_link_libraries(main PRIVATE sha1)