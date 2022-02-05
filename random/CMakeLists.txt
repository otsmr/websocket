
# version of CMake
cmake_minimum_required(VERSION 3.11)

# infos about the project
project(random)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "./build")
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(thread_memory
    thread_memory.cpp
)

target_link_libraries(thread_memory PRIVATE Threads::Threads)