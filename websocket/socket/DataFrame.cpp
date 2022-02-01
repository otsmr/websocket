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

    uint64_t copybytes = (uint64_t) payload_len_bytes - payload_already_received;
    if (copybytes > MAX_PACKET_SIZE)
        copybytes = MAX_PACKET_SIZE;

    uint64_t index;
    for (uint64_t i = 0; i < copybytes; i++) {
        index = i + payload_already_received;
        buffer[i] = buffer[i] ^ (masking_key) >> ((index % 4) * 8);
    }

    memcpy(application_data+payload_already_received, buffer, copybytes);

    payload_already_received += copybytes;

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


    // If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the
    // most significant bit MUST be 0) are the payload length.  
    
    // Multibyte
    // length quantities are expressed in network byte order.  Note that
    // in all cases, the minimal number of bytes MUST be used to encode
    // the length, for example, the length of a 124-byte-long string
    // can't be encoded as the sequence 126, 0, 124.  

    if (frame.payload_len_bytes == 127) {

        std::cout << "Extended Extended payload lenght\n";

        if ((buffer[header_end] >> 7) == 0b1) {
            std::cout << "\nthe most significant bit MUST be 0\n";
            buffer[header_end] = 0;
        }

        frame.payload_len_bytes = (uint64_t) buffer[header_end]   << 56;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+1] << 48;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+2] << 40;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+3] << 32;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+4] << 24;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+5] << 16;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+6] <<  8;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+7]      ;
        header_end += 8;

    }

    if (frame.mask == DataFrame::Bool::Set) {
        frame.masking_key  = (buffer[header_end+0] & 0xff);
        frame.masking_key += (buffer[header_end+1] & 0xff) <<  8;
        frame.masking_key += (buffer[header_end+2] & 0xff) << 16;
        frame.masking_key += (buffer[header_end+3] & 0xff) << 24;
        header_end += 4;
    }

    uint64_t copybytes = frame.payload_len_bytes;
    if (copybytes > (uint64_t) buffer_size - header_end)
        copybytes = (uint64_t) buffer_size - header_end;

    if (frame.mask == DataFrame::Bool::Set) {

        /* 
         * Octet i of the transformed data ("transformed-octet-i") is the XOR of
         * octet i of the original data ("original-octet-i") with octet at index
         * i modulo 4 of the masking key ("masking-key-octet-j"):
         *      j                   = i MOD 4
         *      transformed-octet-i = original-octet-i XOR masking-key-octet-j
         */


        for (uint64_t i = 0; i < copybytes; i++)
            buffer[header_end+i] = buffer[header_end+i] ^ (frame.masking_key) >> ((i % 4) * 8);

    }

    frame.application_data = (uint8_t *) calloc(frame.payload_len_bytes, sizeof(uint8_t));
    if (frame.application_data == NULL) {
        std::cout << "ERROR calloc\n";
        std::cout << "frame.payload_len_bytes=" << frame.payload_len_bytes << "\n";
        exit(1);
        return frame;
    }

    
    memcpy(frame.application_data, buffer+header_end, copybytes);

    frame.payload_already_received = copybytes;

    return frame;

}