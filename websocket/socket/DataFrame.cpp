/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "DataFrame.h"

void DataFrame::add_payload_data(uint8_t buffer[MAX_PACKET_SIZE]) {

    std::cout << "TODO: add_payload_data()\n";

}

void DataFrame::add_fragmented_frame(DataFrame fragmented_frame) {
    std::cout << "TODO: add_fragmented_frame()\n";
}

DataFrame DataFrame::parse_raw_frame(uint8_t buffer[MAX_PACKET_SIZE]) {
    
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

    frame.payload_len_bytes = buffer[1] & 0b1111111;
    uint8_t header_end = 2;

    if (frame.payload_len_bytes == 126) {

        std::cout << "Extended payload lenght\n";

        frame.payload_len_bytes += buffer[header_end] << 8;
        frame.payload_len_bytes += buffer[header_end+1];

        header_end +=2;

    }

    if (frame.payload_len_bytes == 127) {

        if ((buffer[header_end] >> 7) == 0b1) {
            std::cout << "\nthe most significant bit MUST be 0\n";
            buffer[header_end] = 0;
        }
        
        frame.payload_len_bytes += (uint64_t) buffer[header_end]   << 56;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+1] << 48;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+2] << 40;
        frame.payload_len_bytes += (uint64_t) buffer[header_end+3] << 32;
        frame.payload_len_bytes += buffer[header_end+4] << 24;
        frame.payload_len_bytes += buffer[header_end+5] << 16;
        frame.payload_len_bytes += buffer[header_end+6] <<  8;
        frame.payload_len_bytes += buffer[header_end+7];
        header_end += 8;

    }

    if (frame.mask == DataFrame::Bool::Set) {
        frame.masking_key  = (buffer[header_end+0] & 0xff);
        frame.masking_key += (buffer[header_end+1] & 0xff) <<  8;
        frame.masking_key += (buffer[header_end+2] & 0xff) << 16;
        frame.masking_key += (buffer[header_end+3] & 0xff) << 24;
        header_end += 4;
    }

    printf("Bytes (%llu)\n", frame.payload_len_bytes);
    return frame;

    if (frame.mask == DataFrame::Bool::Set) {

        /* 
         * Octet i of the transformed data ("transformed-octet-i") is the XOR of
         * octet i of the original data ("original-octet-i") with octet at index
         * i modulo 4 of the masking key ("masking-key-octet-j"):
         *      j                   = i MOD 4
         *      transformed-octet-i = original-octet-i XOR masking-key-octet-j
         */

        for (uint64_t i = 0; i < frame.payload_len_bytes; i++)
            buffer[header_end+i] = buffer[header_end+i] ^ (frame.masking_key) >> ((i % 4) * 8);

    }

    frame.application_data = (uint8_t *) calloc(frame.payload_len_bytes, sizeof(uint8_t));
    if (frame.application_data == NULL) {
        std::cout << "ERROR calloc\n";
        exit(1);
    }

    
    memcpy(frame.application_data, buffer+header_end, frame.payload_len_bytes);

    /*
      EXAMPLE: For a text message sent as three fragments, the first
      fragment would have an opcode of 0x1 and a FIN bit clear, the
      second fragment would have an opcode of 0x0 and a FIN bit clear,
      and the third fragment would have an opcode of 0x0 and a FIN bit
      that is set. */

    if (frame.opcode == DataFrame::Opcode::TextFrame) {
        std::string s(frame.application_data, frame.application_data+frame.payload_len_bytes);
        frame.text_data = s;
    }

    return frame;

}