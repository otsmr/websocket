use log::{error, info};
use std::io;
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::Mutex;

use crate::http_parser::{self, parse_http_header};

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

enum WSCState {
    Disconnected = 0,
    Closing,
    WaitingForConnection = 10,
    Connected,
    InDataPayload,
}

pub struct WebSocketConnection {
    socket: Arc<Mutex<TcpStream>>,
    state: WSCState,
    pub on_message_fkt: Vec<fn(&mut Self, Message) -> ()>,
}

impl WebSocketConnection {
    pub fn on_message(&mut self, f: fn(&mut Self, Message) -> ()) {
        self.on_message_fkt.push(f);
    }

    pub fn send_message(&mut self, msg: Message) {
        let socket = self.socket.clone();
        // FIXME: is tokio::spawn really necessary?
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
            let n = match socket.read(&mut buf).await {
                Ok(n) if n == 0 => break,
                Ok(n) => n,
                Err(e) => {
                    eprintln!("failed to read from socket; err = {:?}", e);
                    break;
                }
            };

            match self.state {
                WSCState::Connected => (),
                WSCState::WaitingForConnection => {
                    if let Ok(offset) = self.do_handshake(&buf[..n]).await {
                        if offset >= n {
                            continue;
                        }
                    } else {
                        break;
                    };
                }
                _ => break,
            }

            let on_messages = self.on_message_fkt.clone();
            for on_message in on_messages.iter() {
                let msg = Message {
                    kind: MessageKind::String,
                    string: std::str::from_utf8(&buf[..n]).unwrap().to_string(),
                };
                on_message(self, msg);
            }
        }

        // self.close()
        Ok(())
    }

    async fn do_handshake(&self, buf: &[u8]) -> Result<usize, io::Error> {
        let http_header = parse_http_header(buf.to_vec())?;


        let websocket_key = http_header.fields.get("sec-websocket-key");

        // FIMXE: learn how this is done in a better way
        if websocket_key.is_none() || websocket_key.unwrap().len() != 24 {
            return Err(io::Error::new(
                    io::ErrorKind::ConnectionAborted,
                    "not a valid websocket connection request",
            ));
        }
        let websocket_key = websocket_key.unwrap();

        info!("Connected with the key {}", websocket_key);

        Ok(0)
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
    on_connection_fkt: Vec<fn(&mut WebSocketConnection) -> ()>,
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
                on_connection_fkt: vec![],
            });
        }

        Err(io::Error::new(
            io::ErrorKind::InvalidInput,
            "could not resolve to any address",
        ))
    }

    pub fn on_connection(&mut self, f: fn(&mut WebSocketConnection) -> ()) {
        self.on_connection_fkt.push(f);
    }

    pub async fn listen(&self) {
        info!("Waiting for connections!");
        loop {
            match self.listener.accept().await {
                Ok((socket, _addr)) => {
                    let socket = Arc::new(Mutex::new(socket));
                    let mut con = WebSocketConnection {
                        socket,
                        state: WSCState::WaitingForConnection,
                        on_message_fkt: vec![],
                    };
                    let on_connections = self.on_connection_fkt.clone();
                    for on_connection in on_connections.iter() {
                        on_connection(&mut con);
                    }
                    tokio::spawn(async move { con.connect().await });
                }
                Err(e) => error!("couldn't get client: {:?}", e),
            };
        }
    }
}
