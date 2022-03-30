/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <cstdio>
#include <thread>
#include <fstream>
#include <functional>

#include "../http/response.h"
#include "../http/request.h"
#include "../hash/sha1.h"
#include "../base64/base64.h"

#include "frame.h"

#define MAX_PACKET_SIZE 4096
#define CONNECTION_TIMEOUT_SECONDS 5

class WebSocket;

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

    // listens on the socket for messages from the client
    void listen();

    // closes the connection with the client
    void close(bool close_frame_received);
    
    // sends a text message to the client
    void send_message(std::string message);

    State state () const { return m_state; };
    int connection () const { return m_connection; };
    void on_message(fkt_string f) { m_on_message = std::move(f); };

private:

    // file descriptor on the open socket
    int m_connection = -1;

    // state of the current connection
    State m_state { Disconnected };

    // rfc6455 section-7.4 - status codes
    uint16_t m_close_statuscode = 1000;

    // WebSocket extensions used by the current connection (rfc6455 section-9)
    uint8_t m_extensions = NoExtensions;

    // ping was sent, waiting for pong from client
    bool m_waiting_for_pong = false;
    
    // State::InDataPayload -> merge fragmented frames
    std::vector<DataFrame> m_framequeue;

    // function pointer called when a message is received from the client
    fkt_string m_on_message = nullptr;

    // open handshake with client  (rfc6455 section-4.2.2)
    bool handshake(uint8_t buffer[MAX_PACKET_SIZE]);

    // sends a ping to the client every 20s
    void check_for_keep_alive();

    void handle_frame(DataFrame frame);
    void handle_text_frame();
    void send_pong_frame();

};