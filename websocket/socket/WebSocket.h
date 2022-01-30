/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../sha1/sha1.h"
#include "../base64/base64.h"

#define MAX_PACKET_SIZE 1500

class WebSocket {
public:

    enum State {
        Disconnected,
        Closing,
        WaitingForHandshake,
        Connected
    };

    WebSocket(int connection);
    ~WebSocket() {};

    void listen();
    int handshake(uint8_t buffer[MAX_PACKET_SIZE]);
    void close();
    State state () { return m_state; };
    int connection () { return m_connection; };

private:
    State m_state { State::Disconnected };
    int m_connection = -1;

};