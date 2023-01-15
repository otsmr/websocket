mod websocket;
mod dataframe;
mod http_parser;
mod base64;
mod sha1;
use log::info;
// use tokio::io::{AsyncReadExt, AsyncWriteExt};
use websocket::{Message, MessageKind, WebSocket};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");

    let mut ws = WebSocket::bind(vec!["127.0.0.1:3000".parse().unwrap()]).await?;
    // info!("Listening on {}", ws.addr.unwrap());

    ws.on_connection(|wsc| {
        info!("new client connected");

        wsc.on_message(|wsc, message| {
            info!("New Message: {}", message.string.trim());
            wsc.send_message(Message {
                kind: MessageKind::String,
                string: "Hallo Welt!\n".to_string(),
            });
        });
    });

    ws.listen().await;

    Ok(())
}
