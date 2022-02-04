/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <thread>
#include <fstream>

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
    void close(int close_frame_received);
    State state () { return m_state; };
    int connection () { return m_connection; };

    void handle_frame(DataFrame frame);
    int handshake(uint8_t buffer[MAX_PACKET_SIZE]);
    DataFrame create_frame_from_text(std::string text);

private:
    State m_state { State::Disconnected };
    int m_connection = -1;
    int m_close_timeout = 5; // seconds
    int m_waiting_for_pong = 0;
    size_t m_ping_status = 0;
    size_t m_pong_status = 0; // 
    uint16_t m_close_statuscode = 1000;
    std::vector<DataFrame> m_framequeue;

};