
mkdir build
cmake -S . -B build
cmake --build build

if [ "$1" == "run" ]; then
    clear
    echo "[RUN] build/main.bin\n"
    ./build/main.bin
fi