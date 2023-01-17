use log::{error, info};
use std::net::SocketAddr;
use std::{io, vec};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};

use crate::base64;
use crate::dataframe::{DataFrame, Opcode};
use crate::http_parser::{parse_http_header, HttpHeader};
use crate::sha1::sha1;

enum WSCState {
    Disconnected = 0,
    // Closing,
    WaitingForConnection = 10,
    Connected,
    InDataPayload, // with socket.write each time only 1024 bytes
    WaitingForNextFrame,
}

pub struct WebSocketConnection {
    socket: TcpStream,
    state: WSCState,
    send_queue: Vec<DataFrame>,
    pub on_message_fkt: Vec<fn(&mut Self, &String) -> ()>,
}

impl WebSocketConnection {
    pub fn on_message(&mut self, f: fn(&mut Self, &String) -> ()) {
        self.on_message_fkt.push(f);
    }

    pub fn send_message(&mut self, msg: DataFrame) {
        self.send_queue.push(msg);
    }

    pub async fn connect(&mut self) -> io::Result<()> {
        info!("Trying to connect with WebSocket");
        let mut buf = [0; 1024];
        let mut frame_buffer = Vec::<DataFrame>::new();

        loop {
            for df in self.send_queue.iter() {
                if let Err(e) = self.socket.write(df.as_bytes().as_slice()).await {
                    error!("failed to write to the socket: {}", e);
                    // TODO: maybe closing the socket?
                }
            }
            self.send_queue.clear();

            let n = match self.socket.read(&mut buf).await {
                Ok(n) if n == 0 => break,
                Ok(n) => n,
                Err(e) => {
                    eprintln!("failed to read from socket; err = {:?}", e);
                    break;
                }
            };

            match self.state {
                WSCState::InDataPayload => {
                    let len = frame_buffer.len();
                    if len > 0 {
                        frame_buffer[len - 1].add_payload(&buf[..n]);
                    }
                }
                WSCState::Connected | WSCState::WaitingForNextFrame => {
                    let frame = DataFrame::from_raw(&buf[..n]);
                    if frame.is_err() {
                        error!("Got bad dataframe from client: {:?}", &buf[..n]);
                        break;
                    }
                    frame_buffer.push(frame.unwrap());
                }
                WSCState::WaitingForConnection => {
                    let http_response = self.do_handshake(&buf[..n]);
                    if self.socket.write(&http_response.as_vec()).await.is_ok()
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
            if !frame_buffer.is_empty() {
                let frame = frame_buffer.last().unwrap();
                if frame.payload.len() as u64 == frame.payload_size {
                    match frame.opcode {
                        Opcode::TextFrame => {
                            let string = DataFrame::frames_as_string(&frame_buffer);
                            if let Ok(string) = string {
                                let on_messages = self.on_message_fkt.clone();
                                for on_message in on_messages.iter() {
                                    on_message(self, &string);
                                }
                            }
                        }
                        o => log::warn!("Opcode ({:?}) not implemented!", o),
                    }
                    if frame.flags.fin {
                        self.state = WSCState::Connected;
                        frame_buffer.clear();
                    } else {
                        self.state = WSCState::WaitingForNextFrame;
                    }
                } else {
                    self.state = WSCState::InDataPayload;
                }
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
                    let mut con = WebSocketConnection {
                        socket,
                        state: WSCState::WaitingForConnection,
                        on_message_fkt: vec![],
                        send_queue: vec![],
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
