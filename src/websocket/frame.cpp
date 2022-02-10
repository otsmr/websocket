/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "frame.h"

size_t DataFrame::add_payload_data(uint8_t buffer[MAX_PACKET_SIZE], int offset, size_t buffer_size) {

    uint64_t copybytes = (uint64_t) m_payload_len_bytes - m_application_data.size() + offset;

    if (copybytes > buffer_size)
        copybytes = (uint64_t) buffer_size;

    uint64_t index = m_application_data.size();

    for (auto i = (uint64_t) offset; i < copybytes; i++) {

        if (m_mask) {
            buffer[i] = buffer[i] ^ m_masking_key[(index + i - (uint64_t) offset) % 4];
        }

        m_application_data.push_back(buffer[i]);

    }

    return (size_t) copybytes;

}

std::vector<uint8_t> DataFrame::get_raw_frame() {

    uint8_t tmp;
    size_t size = m_application_data.size();
    std::vector<uint8_t> raw_frame;

    tmp  = m_fin << 7;
    tmp |= m_rsv << 4;
    tmp |= m_opcode;

    raw_frame.push_back(tmp);

    tmp = m_mask << 7;

    if (size > 0xffff) {
        tmp |= 127;
        raw_frame.push_back(tmp);

        for (int i = 7; i >= 0; i--) {
            raw_frame.push_back(size >> (i*8));
        }

    } else if (size > 126) {
        tmp |= 126;
        raw_frame.push_back(tmp);

        raw_frame.push_back(size >> (1*8));
        raw_frame.push_back(size);
    } else {
        raw_frame.push_back(tmp | size);
    }

    for (uint8_t& byte : m_application_data)
        raw_frame.push_back(byte);
    
    return raw_frame;

}

size_t DataFrame::parse_raw_frame(uint8_t buffer[MAX_PACKET_SIZE], size_t buffer_size) {
    
    /*
      0               1               2               3
      0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+ */

    uint8_t header_end = 2;
    m_fin = buffer[0] >> 7 == 0x1;

    if (buffer[0] >> 6 == 0x1)
        m_rsv |= DataFrame::RSV::RSV1;
    if (buffer[0] >> 5 == 0x1)
        m_rsv |= DataFrame::RSV::RSV2;
    if (buffer[0] >> 4 == 0x1)
        m_rsv |= DataFrame::RSV::RSV3;

    m_opcode = (DataFrame::Opcode) (buffer[0] & 0b1111);
    m_mask = buffer[1] >> 7 == 0x1;

    //   The length of the "Payload data", in bytes: if 0-125, that is the payload length.

    m_payload_len_bytes = buffer[1] & 0b1111111;

    if (m_payload_len_bytes == 126) {

        // If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length.

        m_payload_len_bytes = (uint16_t) buffer[header_end] << 8;
        m_payload_len_bytes += (uint16_t) buffer[header_end+1];

        header_end += 2;

    }

    if (m_payload_len_bytes == 127) {

        if (buffer[header_end] >> 7) {
            std::cout << "\nthe most significant bit MUST be 0\n";
            buffer[header_end] = 0;
        }

        m_payload_len_bytes = 0;

        for (int i = 0; i < 8; i++) {
            m_payload_len_bytes += (uint64_t) buffer[header_end+i] << (8*(7-i));
        }

        header_end += 8;

    }

    if (m_mask) {
        
        for (int i = 0; i < 4; i++)
            m_masking_key[i] = buffer[header_end+i];

        header_end += 4;

    }

    return add_payload_data(buffer, header_end, buffer_size);

}

DataFrame DataFrame::get_text_frame(std::string string) {

    DataFrame frame;

    frame.m_fin = true;
    frame.m_mask = false;
    frame.m_rsv = 0;
    frame.m_opcode = DataFrame::TextFrame;
    frame.m_application_data = std::vector<uint8_t> (string.begin(), string.end());
    frame.m_payload_len_bytes = frame.m_application_data.size();

    return frame;

}

DataFrame DataFrame::get_ping_frame() {

    DataFrame frame;

    frame.m_fin = true;
    frame.m_mask = false;
    frame.m_rsv = 0;
    frame.m_opcode = DataFrame::Ping;
    frame.m_payload_len_bytes = 0;

    return frame;

}