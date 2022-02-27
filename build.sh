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
    if [ "$2" != "" ]; then
        ctest --output-on-failure --tests-regex "$2"
    else
        ctest --output-on-failure
    fi
fi