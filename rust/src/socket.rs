use std::net::{TcpListener, TcpStream};

#[derive(Debug)]
pub struct SocketClient {
   stream: TcpStream
}

impl SocketClient {
}

#[derive(Debug, Default)]
pub struct Socket {
    port: u32,
    host: String,
    clients: Vec<SocketClient>
}

impl Socket {
    pub fn new() -> Socket {
        Socket {
            port: 8080,
            host: "0.0.0.0".to_string(),
            clients: vec![]
        }
    }
    pub fn listen(&mut self) -> std::io::Result<()> {

        let host = format!("{}:{}", self.host, self.port);

        println!("Listening on {}", host);

        let server = TcpListener::bind(host).unwrap();

        // accept connections and process them serially
        for stream in server.incoming() {
            let sc = SocketClient{stream: stream?};
            self.clients.push(sc);
        }

        Ok(())
    }

}
