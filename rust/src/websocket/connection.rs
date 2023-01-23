use crate::base64;
use crate::dataframe::{DataFrame, Opcode};
use crate::http_parser::{parse_http_header, HttpHeader};
use crate::sha1::sha1;
use log::debug;
use std::io;
use std::io::ErrorKind;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;

#[derive(PartialEq, Eq, Debug)]
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
    request_header: Option<HttpHeader>,
    on_message_fkt: Vec<fn(&mut Self, &String) -> ()>,
    on_close_fkt: Vec<fn(&mut Self, u16, &String) -> ()>,
}

impl Connection {
    pub fn new(socket: TcpStream) -> Self {
        Connection {
            socket,
            state: ConnectionState::WaitingForConnection,
            request_header: None,
            on_message_fkt: vec![],
            on_close_fkt: vec![],
            send_queue: vec![],
        }
    }
    pub fn on_message(&mut self, f: fn(&mut Self, &String) -> ()) {
        self.on_message_fkt.push(f);
    }
    pub fn on_close(&mut self, f: fn(&mut Self, u16, &String) -> ()) {
        self.on_close_fkt.push(f);
    }

    pub fn send_message(&mut self, msg: DataFrame) {
        self.send_queue.push(msg);
    }

    fn send_pong(&mut self, frame: &DataFrame) {
        let mut pong = DataFrame::pong();
        if !frame.payload.is_empty() {
            pong.payload = frame.payload.clone();
        }
        self.send_queue.push(pong);
    }

    pub fn close(&mut self, statuscode: u16) {
        self.state = ConnectionState::CloseFromServer;
        let frame = DataFrame::closing(statuscode);
        self.send_queue.push(frame);
    }

    async fn handle_raw_data(
        &mut self,
        buf: &[u8],
        frames: &mut Vec<DataFrame>,
    ) -> io::Result<(bool, usize)> {
        match self.state {
            ConnectionState::WaitingForFrames => {
                if let Ok(frame) = DataFrame::from_raw(buf) {
                    frames.push(frame);
                    if !frames.last().unwrap().is_full() {
                        self.state = ConnectionState::InDataPayload;
                        return Ok((false, frames.last().unwrap().current_len()));
                    }
                } else {
                    return Ok((false, 0));
                }
            }
            ConnectionState::InDataPayload => {
                let offset = frames.last_mut().unwrap().add_payload(buf);
                if !frames.last().unwrap().is_full() {
                    return Ok((false, offset));
                }
                self.state = ConnectionState::WaitingForFrames;
            }
            ConnectionState::WaitingForConnection => {
                let http_response = self.handle_handshake(buf);
                if self.socket.write_all(&http_response.as_vec()).await.is_ok()
                    && http_response.status_code == 101
                {
                    self.state = ConnectionState::WaitingForFrames;
                    return Ok((false, buf.len()));
                }
                return Err(io::Error::new(
                    ErrorKind::InvalidData,
                    "Invalid handshake request",
                ));
            }
            _ => return Ok((true, 0)),
        }

        let frame = frames.last().unwrap();

        log::info!("Got frame with {:?}", frame.opcode);

        if frame.opcode >= 0x8.into() {
            // Control frames can be interjected in
            // the middle of a fragmented message.
            let frame = frames.pop().unwrap();

            if frame.payload.len() > 125 {
                self.close(1002);
                return Ok((true, buf.len()));
            }

            match frame.opcode {
                Opcode::ConectionClose => {
                    let mut statuscode = frame.get_closing_code();
                    let close_reason;
                    let parse_reason = frame.get_closing_reason();
                    if let Err(error_statuscode) = parse_reason {
                        statuscode = error_statuscode;
                        close_reason = "".to_string();
                    } else {
                        close_reason = parse_reason.unwrap();
                    }
                    if !self.on_close_fkt.is_empty() {
                        let on_closes = self.on_close_fkt.clone();
                        for on_close in on_closes {
                            on_close(self, statuscode, &close_reason);
                        }
                    }
                    if self.state == ConnectionState::CloseFromServer {
                        self.state = ConnectionState::Disconnected;
                        return Ok((true, 0));
                    }
                    let frame = DataFrame::closing(statuscode);
                    self.socket.write_all(frame.as_bytes().as_slice()).await?;

                    self.state = ConnectionState::Disconnected;
                    return Ok((true, 0));
                }
                Opcode::Ping => self.send_pong(&frame),
                Opcode::Pong => (),
                _ => (),
            }
            return Ok((false, frame.current_len()));
        }

        if !frame.flags.fin {
            return Ok((false, frame.current_len()));
        }

        match frame.opcode {
            Opcode::TextFrame => {
                let string = DataFrame::frames_as_string(frames);
                if let Ok(string) = string {
                    let on_messages = self.on_message_fkt.clone();
                    for on_message in on_messages.iter() {
                        on_message(self, &string);
                    }
                } else {
                    log::error!("String Error: {}", string.unwrap_err());
                    self.close(1007);
                    return Ok((true, buf.len()));
                }
            }
            o => log::warn!("Opcode ({:?}) not implemented!", o),
        }

        frames.clear();

        Ok((false, buf.len()))
    }

    pub async fn connect(&mut self) -> io::Result<()> {
        debug!("Trying to connect with WebSocket");

        let mut temp_buffer = vec![];
        let mut read_buffer = [0; 1024];
        let mut frames = Vec::<DataFrame>::new();
        let mut close_connection = false;

        loop {
            for df in self.send_queue.iter() {
                debug!("Send frame {:?}", df.opcode);
                self.socket.write_all(df.as_bytes().as_slice()).await?;
            }

            self.send_queue.clear();

            if close_connection {
                break;
            }

            let rx = self.socket.read(&mut read_buffer);

            let read_buffer_len: usize = if self.state == ConnectionState::CloseFromServer {
                tokio::time::timeout(std::time::Duration::from_secs(2), rx).await??
            } else {
                rx.await?
            };

            if read_buffer_len == 0 {
                self.state = ConnectionState::Disconnected;
                break;
            }

            let mut offset = 0;

            while offset < read_buffer_len && !close_connection {
                let current_offset;

                let mut using_buffer = &read_buffer[offset..read_buffer_len];
                let mut last_offset = 0;

                if !temp_buffer.is_empty() {
                    last_offset = temp_buffer.len();
                    temp_buffer.append(&mut using_buffer.to_vec());
                    using_buffer = temp_buffer.as_slice();
                }

                (close_connection, current_offset) =
                    self.handle_raw_data(using_buffer, &mut frames).await?;

                if current_offset == 0 {
                    // no full dataframe, so safe in temp_buffer
                    if temp_buffer.is_empty() {
                        temp_buffer.append(&mut using_buffer.to_vec());
                    }
                    break;
                } else {
                    offset += current_offset - last_offset;
                    temp_buffer.clear();
                }
            }
        }
        self.socket.shutdown().await?;
        Ok(())
    }
    fn handle_handshake(&mut self, buf: &[u8]) -> HttpHeader {
        let http_header = parse_http_header(buf.to_vec());
        if http_header.is_err() {
            return HttpHeader::response_400();
        }

        let http_header = http_header.unwrap();
        let websocket_key = http_header.fields.get("sec-websocket-key");

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

        self.request_header = Some(http_header);
        response
    }
}
