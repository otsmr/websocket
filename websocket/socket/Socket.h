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

#include "WebSocket.h"

class Socket {
public:
    Socket(std::string url_prefix, int port, int max_connections);

    enum State {
        Running,
        Stopping,
        Stopped
    };

    int listen();
    void display_stats();
    void stop();
    static void threaded_connections(int m_sockfd, sockaddr_in sockaddr, State *state);

private:
    State m_state { State::Running };
    int m_max_connections = 10000;
    int m_sockfd = -1;
    int m_port = 9090;
    std::string m_url_prefix;

};