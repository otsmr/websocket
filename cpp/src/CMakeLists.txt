
# version of CMake
cmake_minimum_required(VERSION 3.11)

# infos about the project
project(
  from-scratch
  VERSION 0.1
  DESCRIPTION "WebSocket from scratch"
  LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "./build")
set(CMAKE_CXX_STANDARD 14)
set(THREADS_PREFER_PTHREAD_FLAG ON)
# add_compile_options("-fno-stack-protector")
include_directories( 
  "./"
  "./socket"
  "./websocket"
  "./http"
  "./base64"
  "./hash"
)
find_package(Threads REQUIRED)

add_executable(wsserver
  wsserver.cpp

  base64/base64.cpp
  
  hash/sha1.cpp
  
  http/http_request.cpp
  http/http_response.cpp
  
  socket/socket.cpp

  websocket/dataframe.cpp
  websocket/websocket.cpp
  
)


target_link_libraries(wsserver PRIVATE Threads::Threads)