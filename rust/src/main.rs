mod base64;
mod dataframe;
mod http_parser;
mod sha1;
mod websocket;
use dataframe::{Opcode, DataFrame};
use log::info;
// use tokio::io::{AsyncReadExt, AsyncWriteExt};
use websocket::WebSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");

    let mut ws = WebSocket::bind(vec!["127.0.0.1:3000".parse().unwrap()]).await?;
    // info!("Listening on {}", ws.addr.unwrap());

    ws.on_connection(|wsc| {
        info!("new client connected");

        wsc.on_message(|wsc, df| {
            info!("New Message: {}", df.as_string().unwrap().trim());
            wsc.send_message(DataFrame::message(
                Opcode::TextFrame,
                "Hallo Welt\n".to_string(),
            ));
        });
    });

    ws.listen().await;

    Ok(())
}
