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

    Socket(int port, int max_connections);

    enum State {
        Running,
        Stopping,
        Stopped
    };

    int listen(int async);
    void stop();

    int port() { return m_port; }

private:

    State m_state { State::Running };
    int m_max_connections = 10000;
    int m_current_connections = 0;
    int m_sockfd = -1;
    int m_port = 9090;

};