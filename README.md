# from scratch
Implementing the WebSocket protocol in C++ from scratch. 

# goal
The primary goal is **to learn C++ on a project**. The goal is to implement the **WebSocket** protocol from scratch. I like to implement everything I need to do this myself, like SHA-1.  
Finally, once I have implemented all the MUST requirements, I want to find security vulnerabilities in my implementation (one is currently becoming a new CTF Challenge :^)), as well as intentionally add a few of my own to make CTFs challenges.

## standards
### already implemented
- [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455)
- [WS: Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [WS: SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [WS: HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
### work in progress
### still to make
- MAYBE: [WS: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- MAYBE: [WS: DEFLATE](https://www.rfc-editor.org/rfc/rfc1951)
- 32 and 64 bit support
- Sockets: IPv6 support 
- Make more stable (add more tests etc.)
- node.js wrapper


# getting started
## requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

## build & run
```
./build.sh run
```

## build & test
```
./build.sh test [sha1]
```

# Security

See [Security.md](SECURITY.md).