/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <iostream>
#include <string>

#include "socket/Socket.h"


int main () {

    char option = 0;
    Socket socket(3001, 10);

    // socket.on("connection", [socket](WebSocket *ws) {
    //     std::cout << "Neue Verbindung [" << ws->connection() << "]\n";

    //     ws->on("message", [=](std::string message) {
    //         std::cout << "Message von [" << ws->connection() << "] :" << message << "\n";
    //     });

    // });

    if (socket.listen(1) == 1)
        return 1;

    while(option != 's') {

        std::cout << "Optionen (s)>"; 

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