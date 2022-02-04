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

#include "DataFrame.h"

#define MAX_PACKET_SIZE 4096

class WebSocket {
public:

    enum State {
        Disconnected,
        Closing,
        WaitingForHandshake = 10, // >= Connected to the WebSocket
        Connected,
        InDataPayload,
    };

    WebSocket(int connection);
    ~WebSocket();

    void listen();
    void close();
    State state () { return m_state; };
    int connection () { return m_connection; };

    void handle_frame(DataFrame frame);
    int handshake(uint8_t buffer[MAX_PACKET_SIZE]);
    DataFrame create_frame_from_text(std::string text);

private:
    State m_state { State::Disconnected };
    int m_connection = -1;
    std::vector<DataFrame> m_framequeue;

};