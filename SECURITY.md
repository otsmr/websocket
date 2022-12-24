


# Fuzzing

[Fuzzing sockets](https://securitylab.github.com/research/fuzzing-sockets-FTP/)

1. change `COMPILE_FOR_FUZZING` flag in `flags.h` to `true`
2. [Creating Target Docker Container](https://github.com/alex-maleno/Fuzzing-Module#how-to-create-target-docker-container)

3. `docker run --rm -it -v "$(pwd)":/fuzz 8cc3066969`
4. Compile for AFL++
    1. `mkdir build_fuzz && cd build_fuzz`
    2. `CC=/AFLplusplus/afl-clang-fast CXX=/AFLplusplus/afl-clang-fast++ cmake ..`
    3. `make`
5. Check
```bash
$ /fuzz/src/build_fuzz/wsserver /fuzz/corpus/con_big
$ /fuzz/src/build_fuzz/wsserver /fuzz/corpus/con_small
```
5. Start AFL++ 
```bash
/AFLplusplus/afl-fuzz -i /fuzz/corpus/ -o out -m none -d -- /fuzz/src/build_fuzz/wsserver
```