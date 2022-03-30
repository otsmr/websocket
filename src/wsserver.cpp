/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */



/*
0 - Emergency (emerg)
1 - Alerts (alert)
2 - Critical (crit)
3 - Errors (err)
4 - Warnings (warn)
5 - Notification (notice)
6 - Information (info)
7 - Debug (debug) 
*/

#define DEBUG_LEVEL 6

#include <iostream>
#include <string>
#include <thread>

#include "socket/socket.h"


int main () {

    int ports[] =  {3000, 3001, 8080, 9090, -1}; // errno: 98 - Address already in use
    int p = 0;

    while (ports[p] != -1)
    {
        
        char option = 0;
        Socket socket(ports[p]);

        socket.on_open([](auto * ws) {

            std::cout << "[WebSocket " << ws->connection() << "] connected\n";

            ws->on_message([ws](std::string message) {

                std::cout << "[WebSocket " << "] Message: " << message << "\n";
                ws->send_message("Hello back!");

            });

        });

        if (socket.listen(true)) {
            
            std::cout << "Port: " << socket.port() << "\n";

            while (option != 's') {

                std::cin >> option;

                if (option == 's')
                    socket.stop();
                else
                    std::cout << "Option nicht bekannt.\n";
                
            }

            break; 
        }

        p++;

    }
    
    
    return 0;

}