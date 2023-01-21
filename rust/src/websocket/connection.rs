use crate::base64;
use crate::dataframe::{DataFrame, Opcode};
use crate::http_parser::{parse_http_header, HttpHeader};
use crate::sha1::sha1;
use log::debug;
use std::io;
use std::io::ErrorKind;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;

#[derive(PartialEq, Eq)]
enum ConnectionState {
    Disconnected = 0,
    CloseFromServer,
    WaitingForConnection = 10,
    WaitingForFrames,
    InDataPayload, // with socket.write each time only 1024 bytes
}

pub struct Connection {
    socket: TcpStream,
    state: ConnectionState,
    send_queue: Vec<DataFrame>,
    on_message_fkt: Vec<fn(&mut Self, &String) -> ()>,
}

impl Connection {
    pub fn new(socket: TcpStream) -> Self {
        Connection {
            socket,
            state: ConnectionState::WaitingForConnection,
            on_message_fkt: vec![],
            send_queue: vec![],
        }
    }
    pub fn on_message(&mut self, f: fn(&mut Self, &String) -> ()) {
        self.on_message_fkt.push(f);
    }

    pub fn send_message(&mut self, msg: DataFrame) {
        self.send_queue.push(msg);
    }

    fn send_pong(&mut self) {
        let pong = DataFrame::pong();
        self.send_queue.push(pong);
    }

    pub fn close(&mut self, statuscode: u16) {
        self.state = ConnectionState::CloseFromServer;
        let frame = DataFrame::closing(statuscode);
        self.send_queue.push(frame);
    }

    pub async fn connect(&mut self) -> io::Result<()> {
        debug!("Trying to connect with WebSocket");

        let mut buf = [0; 1024];
        let mut frame_buffer = Vec::<DataFrame>::new();
        loop {
            for df in self.send_queue.iter() {
                self.socket.write_all(df.as_bytes().as_slice()).await?;
            }

            self.send_queue.clear();

            let n = self.socket.read(&mut buf).await?;
            if n == 0 {
                return Err(io::Error::new(ErrorKind::BrokenPipe, "Socket closed"));
            }

            match self.state {
                ConnectionState::InDataPayload => {
                    let len = frame_buffer.len();
                    if len > 0 {
                        frame_buffer[len - 1].add_payload(&buf[..n]);
                    }
                }
                ConnectionState::WaitingForFrames => {
                    if let Ok(frame) = DataFrame::from_raw(&buf[..n]) {
                        frame_buffer.push(frame);
                    } else {
                        return Err(io::Error::new(
                            ErrorKind::InvalidData,
                            "Error parsing dataframe from client",
                        ));
                    }
                }
                ConnectionState::WaitingForConnection => {
                    let http_response = handle_handshake(&buf[..n]);
                    if self.socket.write_all(&http_response.as_vec()).await.is_ok()
                        && http_response.status_code == 101
                    {
                        self.state = ConnectionState::WaitingForFrames;
                        continue;
                    }
                    return Err(io::Error::new(
                        ErrorKind::InvalidData,
                        "Invalid handshake request",
                    ));
                }
                _ => break,
            }

            if frame_buffer.is_empty() {
                continue;
            }

            let frame = frame_buffer.last().unwrap();

            if frame.payload.len() as u64 != frame.payload_size {
                self.state = ConnectionState::InDataPayload;
                continue;
            }

            self.state = ConnectionState::WaitingForFrames;

            if !frame.flags.fin {
                continue;
            }

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
                Opcode::ConectionClose => {
                    if self.state == ConnectionState::CloseFromServer {
                        self.state = ConnectionState::Disconnected;
                        break;
                    }

                    // If an endpoint receives a Close frame and did not previously send a
                    // Close frame, the endpoint MUST send a Close frame in response.  (When
                    // sending a Close frame in response, the endpoint typically echos the
                    // status code it received.)

                    let frame = DataFrame::closing(frame.get_closing_code());
                    self.socket.write_all(frame.as_bytes().as_slice()).await?;

                    self.state = ConnectionState::Disconnected;
                    break;
                }
                Opcode::Ping => self.send_pong(),
                Opcode::Pong => {}
                o => log::warn!("Opcode ({:?}) not implemented!", o),
            }

            frame_buffer.clear();
        }
        self.socket.shutdown().await?;
        Ok(())
    }
}

fn handle_handshake(buf: &[u8]) -> HttpHeader {
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
