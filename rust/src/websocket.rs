use log::{debug, error};
use std::net::ToSocketAddrs;
use std::sync::Arc;
use std::{io, vec};
use tokio::net::TcpListener;
use tokio::sync::Mutex;

mod connection;
use crate::websocket::connection::Connection;

// #[derive(Debug, PartialEq, Eq)]
// enum WebSocketState {
//     Started = 0,
//     Running,
//     Stopped,
// }

pub struct WebSocket {
    listener: TcpListener,
    open_connection_counter: Arc<Mutex<i64>>,
    on_connection_fkt: Vec<fn(&mut Connection) -> ()>,
}

impl WebSocket {
    pub async fn bind<A>(addr: A) -> io::Result<Self>
    where
        A: ToSocketAddrs,
    {
        let listener = std::net::TcpListener::bind(addr)?;

        Ok(WebSocket {
            listener: TcpListener::from_std(listener)?,
            open_connection_counter: Arc::new(Mutex::new(0)),
            on_connection_fkt: vec![],
        })
    }

    pub fn on_connection(&mut self, f: fn(&mut Connection) -> ()) {
        self.on_connection_fkt.push(f);
    }

    pub async fn listen(&self) {
        let addr = self.listener.local_addr();
        if let Ok(addr) = addr {
            debug!("Waiting for connections at {}!", addr);
        }
        loop {
            debug!(
                "Current {} open connections",
                self.open_connection_counter.lock().await
            );
            match self.listener.accept().await {
                Ok((socket, _addr)) => {
                    let counter_connection = self.open_connection_counter.clone();
                    let on_connections = self.on_connection_fkt.clone();

                    tokio::spawn(async move {
                        *counter_connection.lock().await += 1;
                        let mut con = Connection::new(socket);

                        for on_connection in on_connections.iter() {
                            on_connection(&mut con);
                        }

                        if let Err(e) = con.connect().await {
                            error!("Connection error: {}", e);
                        }
                        let mut c = counter_connection.lock().await;
                        *c -= 1;
                        debug!("Current {} open connections", c);
                    });
                }
                Err(e) => match e.raw_os_error() {
                    Some(24) => {
                        error!("No new sockets could be opened due to an OS limit. `See ulimit -n`")
                    }
                    _ => error!("couldn't get client: {:?}", e),
                },
            };
        }
    }
}
