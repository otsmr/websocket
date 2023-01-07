use log::{error, info};
use std::io;
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::Mutex;

#[derive(Debug)]
pub enum MessageKind {
    String = 0,
    // Binary,
}

#[derive(Debug)]
pub struct Message {
    pub kind: MessageKind,
    pub string: String,
}

pub struct WebSocketConnection {
    socket: Arc<Mutex<TcpStream>>,
    pub on_message_fkt: Option<fn(&mut Self, Message) -> ()>,
}

impl WebSocketConnection {
    pub fn on_message(&mut self, f: fn(&mut Self, Message) -> ()) {
        self.on_message_fkt = Some(f);
    }

    pub fn send_message(&mut self, msg: Message) {
        let socket = self.socket.clone();
        tokio::spawn(async move {
            let mut socket = socket.lock().await;
            socket.write(msg.string.as_bytes()).await;
            // match msg.kind {
            //     MessageKind::String => {
            //     }
            //     // _ => {
            //     //     error!(
            //     //         "Transmitter::send MessageKind {:?} not implemented!",
            //     //         msg.kind
            //     //     );
            //     // }
            // }
        });
    }

    pub async fn connect(&mut self) -> io::Result<()> {
        info!("Trying to connect with WebSocket");
        let mut buf = [0; 1024];

        let socket = Arc::clone(&self.socket);
        // In a loop, read data from the socket and write the data back.
        loop {
            let mut socket = socket.lock().await;
            let _n = match socket.read(&mut buf).await {
                Ok(n) if n == 0 => break,
                Ok(n) => n,
                Err(e) => {
                    eprintln!("failed to read from socket; err = {:?}", e);
                    break;
                }
            };

            if let Some(on_message) = self.on_message_fkt {
                let msg = Message {
                    kind: MessageKind::String,
                    string: std::str::from_utf8(&buf).unwrap().to_string(),
                };

                on_message(self, msg);
            }
        }
        Ok(())
    }
}

// #[derive(Debug, PartialEq, Eq)]
// enum WebSocketState {
//     Started = 0,
//     Running,
//     Stopped,
// }

pub struct WebSocket {
    listener: TcpListener,
    on_connection_fkt: Option<fn(&mut WebSocketConnection) -> ()>,
}

impl WebSocket {
    pub async fn bind(mut addrs: Vec<SocketAddr>) -> io::Result<Self> {
        let mut listener: Option<TcpListener> = None;

        if addrs.is_empty() {
            addrs.push("0.0.0.0:8080".parse().unwrap());
        }

        for addr in addrs {
            if let Ok(ok_listener) = TcpListener::bind(addr).await {
                listener = Some(ok_listener);
                break;
            }
        }

        if let Some(listener) = listener {
            return Ok(WebSocket {
                listener,
                on_connection_fkt: None,
            });
        }

        Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            "could not resolve to any address",
        ))
    }

    pub fn on_connection(&mut self, f: fn(&mut WebSocketConnection) -> ()) {
        self.on_connection_fkt = Some(f);
    }

    pub async fn listen(&self) {
        info!("Waiting for connections!");
        loop {
            match self.listener.accept().await {
                Ok((socket, _addr)) => {
                    let socket = Arc::new(Mutex::new(socket));
                    let mut con = WebSocketConnection {
                        socket,
                        on_message_fkt: None,
                    };
                    if let Some(on_connection) = self.on_connection_fkt {
                        on_connection(&mut con);
                    }
                    tokio::spawn(async move { con.connect().await });
                }
                Err(e) => error!("couldn't get client: {:?}", e),
            };
        }
    }
}
