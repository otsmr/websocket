use log::info;

// use webrocket::dataframe::ControlCloseCode;
use webrocket::websocket::WebSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");

    let mut ws = WebSocket::bind("0.0.0.0:3000").await?;

    ws.on_connection(|wsc| {

        wsc.emit("hello".into(), "world".into());

        wsc.on("howdy".to_string(), |_, msg| {
            info!("{}", msg);
        });

    });

    ws.listen().await;

    Ok(())
}
