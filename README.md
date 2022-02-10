# websocket from scratch
Implementing the WebSocket protocol in C++ from scratch (to learn C++). 

# goals
First of all it is **about learning C++ on a project**. The goal is to implement the **WebSocket** protocol from scratch. I want to implement everything I need for it myself, such as Base64 or SHA-1.  

## standards
### already implemented
- [WS: Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [WS: SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [WS: HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
### work in progress
- [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455)
- Make more stable
### still to make
- [WS: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- [WS: DEFLATE](https://www.rfc-editor.org/rfc/rfc1951)
- [TLSv1.3](https://datatracker.ietf.org/doc/html/rfc8446)
- WebSocket (uint64_t): 32 and 64 bit support
- Sockets: IPv6 support 

# requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

# build
```
cmake --build .
```