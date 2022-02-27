# websocket from scratch
Implementing the WebSocket protocol in C++ from scratch (to learn C++). 

# goals
First of all it is **about learning C++ on a project**. The goal is to implement the **WebSocket** protocol from scratch. I want to implement everything I need for it myself, such as Base64 or SHA-1.  

## standards
### already implemented
- [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455)
- [WS: Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [WS: SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [WS: HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
- [TLSv1.3: SHA256](https://datatracker.ietf.org/doc/html/rfc6234)
- [TLSv1.3: SHA384](https://datatracker.ietf.org/doc/html/rfc6234)
### work in progress
- Make more stable
- [TLSv1.3](https://datatracker.ietf.org/doc/html/rfc8446)
**cipher suites**
- MUST: [TLSv1.3: TLS_AES_128_GCM_SHA256] 
- SHOULD: [TLSv1.3: TLS_AES_256_GCM_SHA384] 
- SHOULD: [TLSv1.3: TLS_CHACHA20_POLY1305_SHA256] RFC8439
**digital signatures**
- MUST: [TLSv1.3: rsa_pkcs1_sha256] (for certificates)
- MUST: [TLSv1.3: rsa_pss_rsae_sha256] (for CertificateVerify and certificates)
- MUST: [TLSv1.3: ecdsa_secp256r1_sha256]
**key exchange**
- MUST: [TLSv1.3: secp256r1] NIST P-256
- SHOULD: [TLSv1.3: X25519] RFC7748
### still to make
- [WS: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- [WS: DEFLATE](https://www.rfc-editor.org/rfc/rfc1951)
- WebSocket (uint64_t): 32 and 64 bit support
- Sockets: IPv6 support 

# requirements
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

# build
```
cmake --build .
```