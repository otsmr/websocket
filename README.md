# from-scratch
Implementing protocols / algotithms from scratch to learn.

# goals
First of all it is **about learning C++ on a project**. The long term goal is to implement a **WebSocket** from scratch. I want to implement everything I need for it myself using the standards, such as Base64 or SHA-1.  

## standards
### already implemented
- [Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
### work in progress
- [HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
### still to make
- [the WebSocket Protocol](https://datatracker.ietf.org/doc/html/rfc6455)
- (?) [URL](https://datatracker.ietf.org/doc/html/rfc3986)
- 32 and 64 bit support

# requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

# build
```
cmake --build .
```