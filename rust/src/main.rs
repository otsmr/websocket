    //                 // socket closed
mod websocket;
use log::info;
// use tokio::io::{AsyncReadExt, AsyncWriteExt};
use websocket::WebSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    info!("Starting WebSocket!");
    let mut ws = WebSocket::new();


    ws.on_connection(|mut wsc| {
        info!("new client connected");

        wsc.on_message(|message| {
            info!("New Message: {}", message);
        });

        wsc
    });


    let wslistener = ws.bind(vec!["127.0.0.1:8080".parse().unwrap()]).await?;
    info!("Listening on {}", ws.addr.unwrap());
    wslistener.listen().await;

    Ok(())
        // let listener = TcpListener::bind("127.0.0.1:8080").await?;

        // loop {
        //     info!("Server startet in loop!");

        //     let (mut socket, _) = listener.accept().await?;
        //     tokio::spawn(async move {
        //         let mut buf = [0; 1024];

        //         // In a loop, read data from the socket and write the data back.
        //         loop {
        //             let n = match socket.read(&mut buf).await {
    //                 Ok(n) if n == 0 => return,
    //                 Ok(n) => n,
    //                 Err(e) => {
    //                     eprintln!("failed to read from socket; err = {:?}", e);
    //                     return;
    //                 }
    //             };

    //             // Write the data back
    //             if let Err(e) = socket.write_all(&buf[0..n]).await {
    //                 eprintln!("failed to write to socket; err = {:?}", e);
    //                 return;
    //             }
    //         }
    //     });
    // }
}
