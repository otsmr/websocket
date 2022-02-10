/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <iostream>
#include <string>
#include <thread>

#include "socket/socket.h"

int main () {

    char option = 0;
    Socket socket(3001);

    socket.on_open([](std::shared_ptr<WebSocket> ws) {

        std::cout << "Neue Verbindung [" << ws->connection() << "]\n";

        ws->on_message([&ws](std::string message) {

            std::cout << "[WebSocket " << ws->connection() << "] Message :" << message << "\n";

        });

    });
    
    if (!socket.listen(true)) {
        
        Socket s(8080);

        if (!s.listen(true))
            return 1;

        socket = s;
        
    };

    std::cout << "Port: " << socket.port() << "\n";

    while (option != 's') {

        std::cin >> option;

        if (option == 's')
            socket.stop();
        else
            std::cout << "Option nicht bekannt.\n";
        
    }

    return 0;

}