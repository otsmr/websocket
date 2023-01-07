use log::{error, info};
use std::io;
use std::net::SocketAddr;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};

#[derive(Debug)]
pub struct WebSocketClient {
    socket: TcpStream,
    on_message_fkt: Option<fn(String) -> ()>,
}

impl WebSocketClient {
    pub async fn connect(&mut self) -> io::Result<()> {
        info!("Trying to connect with WebSocket");
        let mut buf = [0; 1024];

        // In a loop, read data from the socket and write the data back.
        loop {
            let n = match self.socket.read(&mut buf).await {
                Ok(n) if n == 0 => break,
                Ok(n) => n,
                Err(e) => {
                    eprintln!("failed to read from socket; err = {:?}", e);
                    break;
                }
            };

            if let Some(fkt) = self.on_message_fkt {
                fkt(std::str::from_utf8(&buf).unwrap().to_string());
            }

            // Write the data back
            if let Err(e) = self.socket.write_all(&buf[0..n]).await {
                eprintln!("failed to write to socket; err = {:?}", e);
            }
        }
        Ok(())
        // // In a loop, read data from the socket and write the data back.
        // loop {

        //     let _n = match self.socket.read(&mut buf) {
        //         // socket closed
        //         Ok(n) if n == 0 => break,
        //         Ok(n) => n,
        //         Err(e) => {
        //             eprintln!("failed to read from socket; err = {:?}", e);
        //             break;
        //         }
        //     };


        // }
        // Ok(())
    }
    pub fn on_message(&mut self, f: fn(String) -> ()) {
        self.on_message_fkt = Some(f);
    }
}

// #[derive(Debug, PartialEq, Eq)]
// enum WebSocketState {
//     Started = 0,
//     Running,
//     Stopped,
// }

#[derive(Debug)]
pub struct WebSocketListener {
    listener: TcpListener,
    on_connection_fkt: Option<fn(WebSocketClient) -> WebSocketClient>,
}

impl WebSocketListener {
    pub async fn listen(&self) {
        info!("Waiting for connections!");
        loop {
            match self.listener.accept().await {
                Ok((socket, _addr)) => {
                    let fkt_opt = self.on_connection_fkt.clone();
                    tokio::spawn(async move {
                        let mut sc = WebSocketClient {
                            socket,
                            on_message_fkt: None,
                        };
                        if let Some(fkt) = fkt_opt {
                            sc = fkt(sc);
                        }
                        sc.connect().await;
                    });
                }
                Err(e) => error!("couldn't get client: {:?}", e),
            };
        }
    }
}

#[derive(Debug)]
pub struct WebSocket {
    pub addr: Option<SocketAddr>,
    on_connection_fkt: Option<fn(WebSocketClient) -> WebSocketClient>, // clients: Vec<WebSocketClient>,
}

impl WebSocket {
    pub fn new() -> Self {
        WebSocket {
            addr: None,
            on_connection_fkt: None, // clients: vec![],
        }
    }

    pub fn on_connection(&mut self, f: fn(WebSocketClient) -> WebSocketClient) {
        self.on_connection_fkt = Some(f);
    }

    pub async fn bind(&mut self, mut addrs: Vec<SocketAddr>) -> io::Result<WebSocketListener> {
        let mut listener: Option<TcpListener> = None;

        if addrs.is_empty() {
            addrs.push("0.0.0.0:8080".parse().unwrap());
        }

        for addr in addrs {
            if let Ok(ok_listener) = TcpListener::bind(addr).await {
                self.addr = Some(addr);
                listener = Some(ok_listener);
                break;
            }
        }

        if let Some(listener) = listener {
            let on_connection_fkt = self.on_connection_fkt.clone();
            return Ok(WebSocketListener {
                listener,
                on_connection_fkt,
            });
        }

        Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            "could not resolve to any address",
        ))
    }
}
