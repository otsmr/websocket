
# version of CMake
cmake_minimum_required(VERSION 3.11)

# infos about the project
project(
  from-scratch
  VERSION 0.1
  DESCRIPTION "WebSocket from scratch"
  LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "./build")
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(main
  main.cpp

  socket/Socket.cpp
  socket/WebSocket.cpp
  socket/DataFrame.cpp

  sha1/sha1.cpp

  base64/base64.cpp

  http/HttpRequest.cpp
  http/HttpResponse.cpp
  http/Url.h
  
)

target_link_libraries(main PRIVATE Threads::Threads)