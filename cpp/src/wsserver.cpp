/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "flags.h"
#include <iostream>
#include <string>
#include <thread>

#include "socket.h"

#if COMPILE_FOR_FUZZING
char * g_fuzzing_input_file;
#endif


int main (int argc, char *argv[])
{

#if COMPILE_FOR_FUZZING
    g_fuzzing_input_file = argv[argc-1];
    int ports[] =  {3000, -1};
#else
    int ports[] =  {3000, 3001, 8080, 9090, -1}; // errno: 98 - Address already in use
#endif

    int p = 0;

    while (ports[p] != -1)
    {
        
        char option = 0;
        Socket socket(ports[p]);

        socket.on_open([](auto * ws) {

            std::cout << "[WebSocket " << ws->connection() << "] connected\n";

#if ARTIFICIAL_BUGS
            ws->on_message([&](std::string message) {
#else
            ws->on_message([ws](std::string message) {
#endif

                std::cout << "[WebSocket " << ws->connection() << "] Message: " << message << "\n";
                ws->send_message("Hello back!");

            });

        });

#if NOFORK
        
        std::cout << "Port: " << socket.port() << "\n";
        socket.listen();

#else
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
#endif
        p++;

    }
    
    
    return 0;

}