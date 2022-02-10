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

    DataFrame () = default;
    ~DataFrame () = default;

    enum RSV {
        RSV1 = 0b1,
        RSV2 = 0b10,
        RSV3 = 0b100
    };

    enum Opcode {
        ContinuationFrame = 0x0,
        TextFrame = 0x1,
        BinaryFrame = 0x2,
        ConectionClose = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };

    bool m_fin = true;
    bool m_mask = false;

    uint8_t m_rsv{};
    Opcode m_opcode = TextFrame;

    uint64_t m_payload_len_bytes = 0;
    uint8_t m_masking_key[4]{};

    std::vector<uint8_t> m_extensions_data;
    std::vector<uint8_t> m_application_data;

    static DataFrame get_text_frame(std::string string);
    static DataFrame get_ping_frame();

    std::string get_utf8_string() {
        std::string s(m_application_data.begin(), m_application_data.end());
        return s;
    }

    bool payload_full() const { return ((m_payload_len_bytes - m_application_data.size()) == 0); }
    size_t add_payload_data (uint8_t buffer[MAX_PACKET_SIZE], int offset, size_t buffer_size);
    size_t parse_raw_frame (uint8_t buffer[MAX_PACKET_SIZE], size_t buffer_size);
    std::vector<uint8_t> get_raw_frame();
    
};