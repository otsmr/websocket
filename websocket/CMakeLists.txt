
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


add_library(base64 base64/base64.cpp)
add_library(sha1 sha1/sha1.cpp)
add_library(http 
  http/HttpRequest.cpp
  http/HttpResponse.cpp
  http/Url.h
)
add_library(socket 
  socket/Socket.cpp
  socket/WebSocket.cpp
)

add_executable(main
  WebSocketServer.cpp
)

target_link_libraries(main PRIVATE base64)
target_link_libraries(main PRIVATE sha1)
target_link_libraries(main PRIVATE http)
target_link_libraries(main PRIVATE socket)
target_link_libraries(main PRIVATE Threads::Threads)