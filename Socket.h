/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>

#include "WebSocket.h"


class Socket {
public:

    Socket(int port);
    Socket(int port, bool use_tls, int max_connections);
    ~Socket();

    enum State {
        Running,
        Stopping,
        Stopped
    };

    int listen(int async);
    void stop();

    int port() const { return m_port; };

    // use std::shared_ptr ??
    std::function<void(WebSocket *)> on_open = nullptr;


private:

    State m_state { State::Running };
    sockaddr_in m_sockaddr{};

    bool m_use_tls = false;
    int m_max_connections = 10000;
    int m_current_connections = 0;
    int m_sockfd = -1;
    int m_port = 9090;

    void wait_for_connection();

};