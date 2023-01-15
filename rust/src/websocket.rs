use log::{error, info};
use std::net::SocketAddr;
use std::sync::Arc;
use std::{io, vec};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::Mutex;

use crate::dataframe::{DataFrame, Opcode};
use crate::base64;
use crate::http_parser::{parse_http_header, HttpHeader};
use crate::sha1::sha1;


enum WSCState {
    Disconnected = 0,
    // Closing,
    WaitingForConnection = 10,
    Connected,
    // InDataPayload,
}

pub struct WebSocketConnection {
    socket: Arc<Mutex<TcpStream>>,
    state: WSCState,
    pub on_message_fkt: Vec<fn(&mut Self, &DataFrame) -> ()>,
}

impl WebSocketConnection {
    pub fn on_message(&mut self, f: fn(&mut Self, &DataFrame) -> ()) {
        self.on_message_fkt.push(f);
    }

    pub fn send_message(&mut self, msg: DataFrame) {
        let socket = self.socket.clone();
        // FIXME: is tokio::spawn really necessary?
        tokio::spawn(async move {
            let mut socket = socket.lock().await;
            if let Err(e) = socket.write(msg.as_string().unwrap().as_bytes()).await {
                error!("failed to write to the socket: {}", e);
                // TODO: maybe closing the socket?
            }
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
                WSCState::Connected => {
                    let frame = DataFrame::from_raw(&buf[..n]);
                    if frame.is_err() {
                        error!("Got bad dataframe from client: {:?}", &buf[..n]);
                        break;
                    }
                    let frame = frame.unwrap();
                    match frame.opcode {
                        Opcode::TextFrame => {
                            let on_messages = self.on_message_fkt.clone();
                            for on_message in on_messages.iter() {
                                on_message(self, &frame);
                            }
                        }
                        o => log::warn!("Opcode ({:?}) not implemented!", o)
                    }
                }
                WSCState::WaitingForConnection => {
                    let http_response = self.do_handshake(&buf[..n]);
                    if socket.write(&http_response.as_vec()).await.is_ok()
                        && http_response.status_code == 101
                    {
                        self.state = WSCState::Connected;
                        continue;
                    }
                    self.state = WSCState::Disconnected;
                    break;
                }
                _ => break,
            }
        }

        // self.close()
        Ok(())
    }

    fn do_handshake(&self, buf: &[u8]) -> HttpHeader {
        if let Ok(http_header) = parse_http_header(buf.to_vec()) {
            let websocket_key = http_header.fields.get("sec-websocket-key");

            // FIMXE: learn how this is done in a better way
            // I don't want to use unwrap in the next line
            if websocket_key.is_none() || websocket_key.unwrap().len() != 24 {
                return HttpHeader::response_400();
            }

            let mut request_key = websocket_key.unwrap().as_bytes().to_vec();

            request_key.append(&mut b"258EAFA5-E914-47DA-95CA-C5AB0DC85B11".to_vec());

            let response_key = sha1(request_key).to_vec();
            let response_key = base64::encode(&response_key);

            let mut response = HttpHeader::response_101();

            response
                .fields
                .insert("Sec-WebSocket-Accept".to_string(), response_key);

            return response;
        }

        HttpHeader::response_400()
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
        let addr = self.listener.local_addr();
        if let Ok(addr) = addr {
            info!("Waiting for connections at {}!", addr);
        }
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
