/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <iostream>
#include <string>
#include <thread>

#include "socket/Socket.h"


int main () {

    char option = 0;
    Socket socket(3001, 10000);

    std::cout << "hardware_concurrency() = "<< std::thread::hardware_concurrency() << "\n";

    // socket.on("connection", [socket](WebSocket *ws) {
    //     std::cout << "Neue Verbindung [" << ws->connection() << "]\n";

    //     ws->on("message", [=](std::string message) {
    //         std::cout << "Message von [" << ws->connection() << "] :" << message << "\n";
    //     });

    // });

    if (socket.listen(1) == 1) {
        Socket s(8080, 10);
        if (s.listen(1) == 1)
            return 1;
        socket = s;
    }

    std::cout << "Port: " << socket.port() << "\n";

    while (option != 's') {

        std::cout << "Optionen: \n"; 

        std::cin >> option;
        switch (option)
        {
        case 's':
            socket.stop();
            break;
        default:
            std::cout << "Option nicht bekannt.\n";
            break;
        }
    }

    return 0;

}