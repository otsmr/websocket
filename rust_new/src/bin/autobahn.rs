use log::info;

use webrocket::websocket::WebSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::builder()
        .format_timestamp(None)
        .init();
    info!("Starting WebSocket for Autobahn|Testsuite!");

    let mut ws = WebSocket::bind("0.0.0.0:9001").await?;

    ws.on_connection(|wsc| {
        wsc.on_bytes(|wsc, bytes| {
            wsc.send_bytes(bytes);
            // wsc.close(1000);
        });
        wsc.on_message(|wsc, msg| {
            wsc.send_message(msg.to_owned());
            // wsc.close(1000);
        });
    });

    ws.listen().await;

    Ok(())
}
