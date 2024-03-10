# webrocket

WebRocket 🚀 is a WebSocket server library programmed in Rust from
scratch (including SHA-1 and Base64).

**Note: This project was created to learn Rust, so major changes may still occur.**

## installation

To add a library release version from crates.io to a Cargo project,
add this to the 'dependencies' section of your Cargo.toml:

**Please do not use in production yet.**

```toml
webrocket = "0.1.0"
```


## usage

Since I am a big [closure](https://doc.rust-lang.org/book/ch13-01-closures.html)
fan, many functions are offered as clousures, like on_connection or on_message.

To start a simple server it only needs the following code:

```rust

let mut ws = WebSocket::bind("0.0.0.0:3000").await?;

ws.on_connection(|wsc| {
    // send a message to the client
    wsc.emit("hello".into(), "world".into());

    // receive a message from the client
    wsc.on("howdy".to_string(), |_, msg| {
        info!("{}", msg); // prints "stranger"
    });

});

ws.listen().await;

```
The client can be started using [webrocket-client](https://github.com/otsmr/webrocket-client).

```javascript
const socket = new WebRocket("ws://127.0.0.1:3000");

// receive a message from the server
socket.on("hello", (msg) => {
  console.log("hello", msg); // prints "world"
});

// send a message to the server
socket.emit("howdy", "stranger");
````

## tests

The server implementation was tested with the [Autobahn|Testsuite](https://github.com/crossbario/autobahn-testsuite)
as follows:

```bash
$ RUST_LOG=debug cargo run --bin wsserver_autobahn

$ docker run -it --rm --net=host \
  -v "${PWD}/tests:/config" \
  -v "${PWD}/tests/reports:/reports" \
  --name fuzzingclient \
  crossbario/autobahn-testsuite \
  wstest -m fuzzingclient --spec /config/fuzzingclient.json
```

There are also tests in the code, which can be started with `cargo test`.

## standards
### already implemented
- [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455)
- [WS: Base64](https://datatracker.ietf.org/doc/html/rfc4648#section-4)
- [WS: SHA-1](https://datatracker.ietf.org/doc/html/rfc3174)
- [WS: HTTP-Header](https://datatracker.ietf.org/doc/html/rfc2616)
<!-- ### work in progress -->
### still to make
- [WS: permessage-deflate](https://www.rfc-editor.org/rfc/rfc7692.html)
- [WS: DEFLATE](https://www.rfc-editor.org/rfc/rfc1951)
- TLS v1.3 (I'm currently implementing TLSv1.3 in [this](https://github.com/otsmr/anothertls) repo.)


## Side project

Before I wrote the WebSocket in Rust, I first programmed it in C++ to teach
myself C++. I now use this implementation, which is in the `fun-with-cpp` branch,
as a playground for e.g. fuzzing.


### requirements (cpp)
- CMake 3.22.1 `brew install cmake`
- A C++17 compatibler compiler

**check compile options in flags.h (!)**

```c
#define COMPILE_FOR_FUZZING 0
#define ARTIFICIAL_BUGS     0
```

### build & run
```
./build.sh run
```

### build & test
```
./build.sh test [sha1]
```
