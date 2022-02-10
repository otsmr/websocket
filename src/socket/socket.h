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

#include "../websocket/websocket.h"

typedef std::function<void(std::shared_ptr<WebSocket>)> fkt_ws;


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

    bool listen(bool async);
    void stop();

    int port() const { return m_port; };

    void on_open(fkt_ws f) { m_on_open = f; };

private:

    fkt_ws m_on_open = nullptr;
    
    State m_state { State::Running };
    sockaddr_in m_sockaddr{};

    bool m_use_tls = false;
    int m_max_connections = 10000;
    int m_current_connections = 0;
    int m_sockfd = -1;
    int m_port = 9090;

    void wait_for_connection();

};