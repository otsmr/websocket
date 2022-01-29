/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

class WebSocket {
public:

    enum State {
        Disconnected,
        Closing,
        Connected
    };

    WebSocket(int connection);

    void listen();
    void close();
    State state () { return m_state; };
    int connection () { return m_connection; };

private:
    State m_state { State::Disconnected };
    int m_connection = -1;

};