/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>

#define MAX_PACKET_SIZE 4096

#ifdef __linux__
typedef u_int8_t uint8_t;
#endif

// #include "WebSocket.h"

class DataFrame {
public:

    DataFrame ();
    ~DataFrame ();

    enum Bool {
        NotSet = 0b0,
        Set = 0b1
    };

    Bool m_fin;
    Bool m_mask;

    enum RSV {
        RSV1 = 0b1,
        RSV2 = 0b10,
        RSV3 = 0b100
    };
    uint8_t m_rsv;
    enum Opcode {
        ContinuationFrame = 0x0,
        TextFrame = 0x1,
        BinaryFrame = 0x2,
        ConectionClose = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };
    Opcode m_opcode;

    // "Extension data" + the length of the "Application data"
    // The length of the "Extension data" may be
    // zero, in which case the payload length is the length of the
    // "Application data"
    uint64_t m_payload_len_bytes = 0; // max 64 bits in length
    
    uint8_t m_masking_key[4];

    std::vector<uint8_t> m_extensions_data;
    std::vector<uint8_t> m_application_data;

    std::string get_utf8_string() {
        std::string s(m_application_data.begin(), m_application_data.end());
        return s;
    }

    int payload_full() { return ((m_payload_len_bytes - m_application_data.size()) == 0) ? 1 : 0; }
    size_t add_payload_data (uint8_t buffer[MAX_PACKET_SIZE], int offset, int buffer_size);
    int parse_raw_frame (uint8_t buffer[MAX_PACKET_SIZE], int buffer_size);
    std::vector<uint8_t> get_raw_frame();
    
};