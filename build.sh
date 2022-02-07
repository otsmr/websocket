#!/bin/bash

if [ "$1" == "test" ]; then
    cd ./tests
fi

mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    
    echo "-----------------------"
    pkill main
    ./build/main
    echo ""

fi

if [ "$1" == "test" ]; then
    cd build
    ctest
fi