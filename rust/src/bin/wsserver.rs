use log::info;

use websocket::dataframe::ControlCloseCode;
use websocket::websocket::WebSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");

    let mut ws = WebSocket::bind("0.0.0.0:3000").await?;

    ws.on_connection(|wsc| {
        info!("New connection");

        wsc.on_message(|wsc, msg| {
            info!("New message: {}", msg);
            wsc.send_message(msg);
            if msg == &"close".to_string() {
                wsc.close(ControlCloseCode::Normal);
            }
        });

        wsc.on_close(|_, code, reason| {
            info!("Connection closed ({:?}) with '{}' as reason.", code, reason);
        });
    });

    ws.listen().await;

    Ok(())
}
