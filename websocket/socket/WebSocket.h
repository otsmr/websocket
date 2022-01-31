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

class DataFrame {
public:

    enum Bool {
        NotSet = 0,
        Set = 1
    };

    Bool fin;
    Bool mask;

    enum RSV {
        RSV1 = 0b1,
        RSV2 = 0b10,
        RSV3 = 0b100
    };
    uint8_t rsv;
    enum Opcode {
        ContinuationFrame = 0x0,
        TextFrame = 0x1,
        BinaryFrame = 0x2,
        ConectionClose = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };
    Opcode opcode;

    // "Extension data" + the length of the "Application data"
    // The length of the "Extension data" may be
    // zero, in which case the payload length is the length of the
    // "Application data"
    uint64_t payload_len_bytes = 0; // max 64 bits in length
    
    uint32_t masking_key;

    uint8_t *extensions_data;
    uint8_t *application_data;

    std::string text_data;
    
};

class WebSocket {
public:

    enum State {
        Disconnected,
        Closing,
        WaitingForHandshake,
        InDataFrame,
        WaitingForNewFrame
    };

    WebSocket(int connection);
    ~WebSocket() {};

    void listen();
    int handshake(uint8_t buffer[MAX_PACKET_SIZE]);
    static DataFrame parse_data_frame(uint8_t buffer[MAX_PACKET_SIZE]);
    void close();
    State state () { return m_state; };
    int connection () { return m_connection; };

private:
    State m_state { State::Disconnected };
    int m_connection = -1;

};