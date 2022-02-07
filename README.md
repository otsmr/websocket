# websocket from scratch
Implementing the WebSocket protocol in C++ from scratch (to learn C++). 

# goals
First of all it is **about learning C++ on a project**. The goal is to implement the **WebSocket** protocol from scratch. I want to implement everything I need for it myself, such as Base64 or SHA-1.  

## standards
### already implemented
- [Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [HTTP-Header parser](https://datatracker.ietf.org/doc/html/rfc2616)
- [the WebSocket Protocol](https://datatracker.ietf.org/doc/html/rfc6455)
### work in progress
- Sockets
### still to make
- [extension: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- [DEFLATE Compressed Data Format](https://www.rfc-editor.org/rfc/rfc1951)
- [extensions](https://www.iana.org/assignments/websocket/websocket.xhtml)
- 32 and 64 bit support
- TLS Wrapper für den Socket

# requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

# build
```
cmake --build .
```