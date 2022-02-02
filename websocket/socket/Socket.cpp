/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "Socket.h"

Socket::Socket(int port, int max_connections) {
    m_max_connections = max_connections;
    m_port = port;
}

void Socket::stop() {

    m_state = Socket::State::Stopping;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET6

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(m_port); 

    int close_thread = connect(sockfd, (struct sockaddr*)& sockaddr, sizeof(sockaddr));

    while (1)
    {
        if (m_state == State::Stopped)
            break;
        sleep(1);
    }

    close(close_thread);
    close(m_sockfd);

}

int Socket::listen (int async) {   

    // TODO: AF_INET6
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        return 1;
    } 

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(m_port); 
    
    if (bind(m_sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port " << m_port << ". errno: " << errno << std::endl;
        return 1;
    }

    if (::listen(m_sockfd, m_max_connections) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        return 1;
    }

    int *sockfd = &m_sockfd;
    State *state = &m_state;

    auto connect = [=]() {

        while (1) {

            auto addrlen = sizeof(sockaddr);
        
            int connection = accept(*sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
            if (connection < 0) {
                std::cout << "Failed to grab connection. errno: " << errno << std::endl;
                return;
            }

            if (*state != Socket::State::Running) {

                // std::cout << "Stopping WebSockets\n";

                // for (WebSocket * connection : connections)
                //     connection->close();

                // int run = 1;
                // while (run)
                // {
                //     run = 0;
                //     for (WebSocket *connection : connections)
                //         if (connection->state() != WebSocket::State::Disconnected)
                //             run = 1;
                //     sleep(1); 
                // }

                // for (WebSocket *connection : connections)
                //     connection->~WebSocket();

                // std::cout << "WebSockets stoppend\n";

                *state = State::Stopped;
                return;
            }

            // int free_con = -1;

            // for(int i = 0; i < connections.size(); i++) {
            //     if (connections[i]->state() == WebSocket::State::Disconnected) {
            //         free_con = i;
            //         break;
            //     }
            // }

            // if (free_con == -1 && *max_connections <= *current_connections) {
            //     std::cout << "Maximum number of connections reached.\n";
            //     close(connection);
            //     continue;
            // }


            WebSocket * webSocket = new WebSocket (connection);       

            // if (free_con > -1) {
                // ???
                // main(40456,0x16d253000) malloc: *** error for object 0x60000126c000: pointer being freed was not allocated
                // main(40456,0x16d253000) malloc: *** set a breakpoint in malloc_error_break to debug
                // ./build.sh: line 20: 40456 Abort trap: 6           ./build/main
            //     connections[free_con]->~WebSocket();
            //     *connections[free_con] = *webSocket;
            // } else {
            //     (*current_connections)++;
            //     connections.push_back(webSocket);
            // }

            std::thread ([&]() {
                webSocket->listen();
            }).detach();
            
        }
        
    };
    
    if (async) {
        std::thread (connect).detach();
    } else {
        connect();
    }

    return 0;

}