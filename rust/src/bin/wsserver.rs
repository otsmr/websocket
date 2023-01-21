use log::info;

use websocket::websocket::WebSocket;
use websocket::dataframe::DataFrame;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");

    let mut ws = WebSocket::bind("0.0.0.0:3001").await?;
    // info!("Listening on {}", ws.addr.unwrap());

    ws.on_connection(|wsc| {
        info!("new client connected");

        wsc.on_message(|wsc, msg| {
            info!("New Message: {}", msg);
            wsc.send_message(DataFrame::text("Hello Back!\n".to_string()));

            if msg == &"close".to_string() {
                wsc.close(1000);
            }
        });

    });

    ws.listen().await;

    Ok(())
}
