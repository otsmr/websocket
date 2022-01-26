
mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    
    echo "\n ------------ \n\n[RUN] build/main.bin\n"
    ./build/main.bin

    echo "\n"
fi