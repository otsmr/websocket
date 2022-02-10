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
#include <cstdio>
#include <thread>
#include <fstream>

#include "../http/response.h"
#include "../http/request.h"
#include "../sha1/sha1.h"
#include "../base64/base64.h"

#include "frame.h"

#define MAX_PACKET_SIZE 4096

typedef std::function<void(std::string)> fkt_string;

class WebSocket {
public:

    explicit WebSocket(int connection);
    ~WebSocket() = default;

    enum State {
        Disconnected,
        Closing,
        WaitingForHandshake = 10, // >= Connected to the WebSocket
        Connected,
        InDataPayload,
    };
    enum Extensions {
        NoExtensions = 0b0,
        PermessageDeflate = 0b1
    };

    void listen();
    void close(bool close_frame_received);
    
    State state () const { return m_state; };
    int connection () const { return m_connection; };

    void on_message(fkt_string f) { m_on_message = f; };

private:

    fkt_string m_on_message = nullptr;

    State m_state { Disconnected };
    uint8_t m_extensions = NoExtensions;

    int m_connection = -1;
    int m_close_timeout = 5; // seconds
    int m_waiting_for_pong = 0;
    
    size_t m_ping_status = 0;
    size_t m_pong_status = 0; // 
    uint16_t m_close_statuscode = 1000;
    std::vector<DataFrame> m_framequeue;

    void handle_frame(DataFrame frame);
    bool handshake(uint8_t buffer[MAX_PACKET_SIZE]);
    void listen_from_client();

};