/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <iostream>
#include <string>
#include "base64/base64.h"
#include "sha1/sha1.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include "socket/Socket.h"

int main () {

    char option = 0;
    Socket socket((std::string) "/", 8080, 10000);

    int status = socket.listen();
    if (status == 1)
        return 1;

    while(option != 's') {
        std::string o = 
            "-- Optionen -- \n" \
            "   s: Server stoppen\n" \
            "   a: Status anzeigen\n";

        std::cout << o; 
        std::cin >> option;
        switch (option)
        {
        case 's':
            socket.stop();
            break;
        case 'a':
            socket.display_stats();
            break;
        default:
            std::cout << "Option nicht bekannt.\n";
            break;
        }
    }

    return 0;

}