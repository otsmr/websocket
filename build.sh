

if [ "$1" == "test" ]; then
    cd ./tests
else
    cd ./websocket
fi

mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    
    echo "\n ------------ \n\n[RUN] websocket server\n"
    ./build/main

    echo "\n"
fi

if [ "$1" == "test" ]; then
    cd build
    ctest
fi