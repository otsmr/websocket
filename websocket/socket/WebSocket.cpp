/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "WebSocket.h"

WebSocket::WebSocket(int connection) {
    m_connection = connection;
}

void WebSocket::listen() {

    std::cout << "[WebSocket " << m_connection << "] open\n";

    m_state = State::Connected;

    while (1) {

        if (m_state != State::Connected)
            break;

        // Read from the connection
        char buffer[100];
        auto bytesRead = read(m_connection, buffer, 100);
        std::cout << "[WebSocket " << m_connection << "] Message: " << buffer;

        // Send a message to the connection
        std::string response = "Good talking to you\n";
        send(m_connection, response.c_str(), response.size(), 0);

    }

}

void WebSocket::close() {

    m_state = State::Closing;
    // Close WebSocket  ...
    std::cout << "[WebSocket " << m_connection << "] closed\n";
    ::close(m_connection);
    m_state = State::Disconnected;

}