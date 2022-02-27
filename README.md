# from scratch
Implementing the WebSocket and TLSv1.3 protocol in C++ from scratch. 

# goals
The primary goal is **to learn C++ on a project**. The goal is to implement the **WebSocket** and the **TLSv1.3** protocol from scratch. I like to implement everything I need to do this myself, like SHA-2 or AES.  
Finally, once I have implemented all the MUST requirements, I want to find security vulnerabilities in my implementation (there will definitely be some :/ ), as well as intentionally add a few of my own (like [Heartbleed](https://en.wikipedia.org/wiki/Heartbleed)) to make CTFs challenges :^).

## standards
### already implemented
- [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455)
- [WS: Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [WS: SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [WS: HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
- [TLS: SHA256](https://datatracker.ietf.org/doc/html/rfc6234)
- [TLS: SHA384](https://datatracker.ietf.org/doc/html/rfc6234)
### work in progress
- [TLSv1.3](https://datatracker.ietf.org/doc/html/rfc8446) (with [Modern compatibility](https://wiki.mozilla.org/Security/Server_Side_TLS))
- Certificate type: [TLS: ECDSA (P-256)]()
- Cipher suite: [TLS: AES_128_GCM]()
- Cipher suite: [TLS: AES_256_GCM]()
- Cipher suite: [TLS: HACHA20_POLY1305]()
- TLS curves: [TLS: X25519]()
- TLS curves: [TLS: prime256v1]()
- TLS curves: [TLS: secp384r1]()
### still to make
- MAYBE: [WS: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- MAYBE: [WS: DEFLATE](https://www.rfc-editor.org/rfc/rfc1951)
- 32 and 64 bit support
- Sockets: IPv6 support 
- Make more stable (add more tests etc.)

# requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

# build
```
cmake --build .
```