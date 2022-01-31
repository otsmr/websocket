/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "Socket.h"

Socket::Socket(std::string url_prefix, int port, int max_connections) {
    m_max_connections = max_connections;
    m_port = port;
    m_url_prefix = url_prefix;
}

void Socket::display_stats() {

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


void Socket::threaded_connections (int m_sockfd, sockaddr_in sockaddr, State *state) {

    std::vector<WebSocket> connections;

    while (1) {

        auto addrlen = sizeof(sockaddr);
    
        int connection = accept(m_sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        if (connection < 0) {
            std::cout << "Failed to grab connection. errno: " << errno << std::endl;
            return;
        }

        if (*state != Socket::State::Running) {

            std::cout << "Stopping WebSockets\n";

            for (WebSocket& connection : connections)
                connection.close();

            int run = 1;
            while (run)
            {
                run = 0;
                for (WebSocket& connection : connections)
                    if (connection.state() != WebSocket::State::Disconnected)
                        run = 1;
                sleep(1); 
            }

            for (WebSocket& connection : connections)
                connection.~WebSocket();

            std::cout << "WebSockets stoppend\n";

            *state = State::Stopped;
            return;
        }

        WebSocket * p = (WebSocket *) calloc(1, sizeof(WebSocket));
        WebSocket * webSocket = new (p) WebSocket (connection);

        connections.push_back(*webSocket);
        // CHECK: unsicher?
        std::thread ([](WebSocket *webSocket) {
            webSocket->listen();
        }, webSocket).detach();

        // TODO: Was, wenn der WebSocket von Client geschlossen wird?
        
    }
    
};

int Socket::listen () {   

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

    // TODO: max connections
    std::thread (threaded_connections, m_sockfd, sockaddr, &m_state).detach();
    std::cout << "Startet";

    return 0;

}