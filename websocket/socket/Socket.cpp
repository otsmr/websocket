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

void Socket::wait_for_connection () {

    auto addrlen = sizeof(m_sockaddr);

    while (1) {

        int connection = accept(m_sockfd, (struct sockaddr*)&m_sockaddr, (socklen_t*)&addrlen);
        if (connection < 0) {
            std::cout << "Failed to grab connection. errno: " << errno << std::endl;
            return;
        }

        if (m_state != Socket::State::Running) {

            std::cout << "Stopping WebSockets\n";

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

            std::cout << "WebSockets stoppend\n";

            m_state = State::Stopped;
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

        std::cout << "connection=" <<  connection << std::endl;

        WebSocket * webSocket = new WebSocket (connection);
        webSocket->listen();
        
    }

}

int Socket::listen (int async) {   

    // TODO: AF_INET6
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        return 1;
    } 

    m_sockaddr.sin_family = AF_INET;
    m_sockaddr.sin_addr.s_addr = INADDR_ANY;
    m_sockaddr.sin_port = htons(m_port); 
    
    if (bind(m_sockfd, (struct sockaddr*)&m_sockaddr, sizeof(m_sockaddr)) < 0) {
        std::cout << "Failed to bind to port " << m_port << ". errno: " << errno << std::endl;
        return 1;
    }

    if (::listen(m_sockfd, m_max_connections) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        return 1;
    }

    if (async) {
        std::thread ([&]() {
            wait_for_connection();
        }).detach();
    } else {
        wait_for_connection();
    }

    return 0;

}