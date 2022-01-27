

if [ "$1" == "test" ]; then
    cd ./tests
fi


mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    
    echo "\n ------------ \n\n[RUN] build/main.bin\n"
    ./build/main.bin

    echo "\n"
fi

if [ "$1" == "test" ]; then
    cd build
    ctest
fi