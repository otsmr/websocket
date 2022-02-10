#!/bin/bash

if [ "$1" == "test" ]; then
    cd ./tests
else
    cd ./src
fi

mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    
    echo "-----------------------"
    pkill wsserver
    ./build/wsserver
    echo ""

fi

if [ "$1" == "test" ]; then
    cd build
    ctest
fi