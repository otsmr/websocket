/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "DataFrame.h"

DataFrame::DataFrame ()
{
}
DataFrame::~DataFrame ()
{
}

size_t DataFrame::add_payload_data(uint8_t buffer[MAX_PACKET_SIZE]) {

    uint64_t copybytes = (uint64_t) payload_len_bytes - application_data.size();

    if (copybytes > MAX_PACKET_SIZE)
        copybytes = MAX_PACKET_SIZE;

    uint64_t index = application_data.size();

    for (uint64_t i = 0; i < copybytes; i++)
        buffer[i] = buffer[i] ^ masking_key[(index + i) % 4];

    for (uint64_t i = 0; i < copybytes; i++)
        application_data.push_back(buffer[i]);

    return (size_t) copybytes;

}

DataFrame DataFrame::parse_raw_frame(uint8_t buffer[MAX_PACKET_SIZE], int buffer_size) {
    
    /*
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1

      0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
      7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0
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

    DataFrame frame;
    int i;

    frame.fin = (buffer[0] >> 7 == 0x0) ? DataFrame::Bool::NotSet : DataFrame::Bool::Set;

    if (buffer[0] >> 6 == 0x1)
        frame.rsv |= DataFrame::RSV::RSV1;
    if (buffer[0] >> 5 == 0x1)
        frame.rsv |= DataFrame::RSV::RSV2;
    if (buffer[0] >> 4 == 0x1)
        frame.rsv |= DataFrame::RSV::RSV3;

    uint8_t opcode = buffer[0] & 0b1111;
    frame.opcode = (DataFrame::Opcode) opcode;

    frame.mask = (buffer[1] >> 7 == 0x0) ? DataFrame::Bool::NotSet : DataFrame::Bool::Set;


    uint8_t header_end = 2;

    //   The length of the "Payload data", in bytes: if 0-125, that is the
    //   payload length.

    frame.payload_len_bytes = buffer[1] & 0b1111111;

    if (frame.payload_len_bytes == 126) {

        // If 126, the following 2 bytes interpreted as a
        // 16-bit unsigned integer are the payload length.

        std::cout << "Extended payload lenght\n";

        frame.payload_len_bytes = (uint16_t) buffer[header_end] << 8;
        frame.payload_len_bytes += (uint16_t) buffer[header_end+1];

        header_end +=2;

    }

    if (frame.payload_len_bytes == 127) {

        std::cout << "Extended Extended payload lenght\n";

        if ((buffer[header_end] >> 7) == 0b1) {
            std::cout << "\nthe most significant bit MUST be 0\n";
            buffer[header_end] = 0;
        }

        frame.payload_len_bytes = 0;

        for (i = 0; i < 8; i++)
            frame.payload_len_bytes += (uint64_t) buffer[header_end+i] << (8*(7-i));
        
        header_end += 8;

    }

    if (frame.mask == DataFrame::Bool::Set) {
        
        for (i = 0; i < 4; i++)
            frame.masking_key[i] = buffer[header_end+i];

        header_end += 4;
    }

    uint64_t copybytes = frame.payload_len_bytes;
    if (copybytes > (uint64_t) buffer_size - header_end)
        copybytes = (uint64_t) buffer_size - header_end;

    if (frame.mask == DataFrame::Bool::Set) {

        for (uint64_t i = 0; i < copybytes; i++)
            buffer[header_end+i] = buffer[header_end+i] ^ frame.masking_key[i % 4];

    }

    for (uint64_t i = 0; i < copybytes; i++)
        frame.application_data.push_back(buffer[header_end+i]);


    return frame;

}