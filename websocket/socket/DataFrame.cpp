/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "DataFrame.h"

DataFrame::DataFrame () = default;
DataFrame::~DataFrame () = default;

size_t DataFrame::add_payload_data(uint8_t buffer[MAX_PACKET_SIZE], int offset, size_t buffer_size) {

    uint64_t copybytes = (uint64_t) m_payload_len_bytes - m_application_data.size() + offset;

    if (copybytes > (uint64_t) buffer_size)
        copybytes = (uint64_t) buffer_size;

    uint64_t index = m_application_data.size();

    int nulls = 0;
    for (uint64_t i = (uint64_t) offset; i < copybytes; i++) {
        nulls = 0;

        /*
         *   db7388cf df778cc3 d37b80c7 d77f84db cb6398df cf679cd3 c36beba3 ab02fbbe 
         *   ad03e281 db7388cf df778cc3 d37b80c7 d77f84db cb6398df cf679cd3 c36beba3 
         *   ab02fbbd aa09e281 __db7388cf__ df778cc3 d37b80c7 d77f84db cb6398df cf679cd3 
         *   c36beba3 ab02fbbd ae05e281 __db73 [ 0x00 * 402 ]
         *
         *   88cf__ df77 8cc3d37b 80c7d77f 84dbcb63 98dfcf67 9cd3c36b eba3ab02 fbbda201 
         *   e281db73 88cfdf77 8cc3d37b 80c7d77f 84dbcb63 98dfcf67 9cd3c36b eba3ab02 
         *   fbbcab07 e281db73 88cfdf77 8cc3d37b 80c7d77f 84dbcb63 98dfcf67 9cd3c36b 
         */
        
        /*
            ----------------------------------------------------- 
            818a8567 6bd8f50e 05bfa517 a86eeb [ 0x00 * 4081 ]

            XXXXXX -> offset = 0
            ----------------------------------------------------- 
            818a3c8a c53b4ce3 ab5c1cfa 068d52ed  [ 0x00 * 4080 ]
            */

        if (m_mask == Bool::Set) {
            // offset = header_end -> index starts at data payload
            buffer[i] = buffer[i] ^ m_masking_key[(index + i - (uint64_t) offset) % 4];
        }

        m_application_data.push_back(buffer[i]);

    }

    return (size_t) copybytes;

}

std::vector<uint8_t> DataFrame::get_raw_frame() {

    std::vector<uint8_t> raw_frame;

    uint8_t tmp;
    tmp = m_fin << 7;
    tmp |= m_rsv << 4;
    tmp |= m_opcode;
    raw_frame.push_back(tmp);

    tmp = m_mask << 7; // not masked
    if (m_application_data.size() > 65536) {
        // 16 bit => 
        tmp |= 127;
        raw_frame.push_back(tmp);
        uint64_t size = m_application_data.size();

        for (size_t i = 7; i >= 0; i--)
        {
            tmp = size >> (i*8);
            raw_frame.push_back(tmp);
        }

    }
    else if (m_application_data.size() > 126) {
        tmp |= 126;
        raw_frame.push_back(tmp);
        uint16_t size = m_application_data.size();

        for (size_t i = 1; i >= 0; i--)
        {
            tmp = size >> (i*8);
            raw_frame.push_back(tmp);
        }
        
    } else {
        
        tmp |= m_application_data.size();
    }
    raw_frame.push_back(tmp);
    for (uint8_t& byte : m_application_data) {
        raw_frame.push_back(byte);
    }

    // std::fstream f;
    // f.open("buffer.txt", std::ios::app);

    // f << "\n";

    // for (size_t i = 0; i < raw_res.size(); i++)
    // {
    //     char hex[4];
    //     snprintf(hex, 4, "%02x", raw_res.at(i));
    //     f << hex[0] << hex[1];
    //     if ((i+1) % 4 == 0)
    //         f << " ";
    //     if ((i+1) % (4*8) == 0)
    //         f << "\n";
    // }

    // f.close();
    
    return raw_frame;

}

int DataFrame::parse_raw_frame(uint8_t buffer[MAX_PACKET_SIZE], size_t buffer_size) {
    
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

    /*
        [Header]
            81 ff 00 00
            00 00 00 15
            3d ad 53 dd
            f4 53 [ 0x00 * 4082 ]
        
        [Daten]
        XXXXXX -> offset = 0
        ----------------------------------------------------- 
        129fb717 169bb31b 1a97bf1f 1e93bb03 028fa707 068ba30b 0a87d47b 61ebdd59 
        129fb717 169bb31b 1a97bf1f 1e93bb03 028fa707 068ba30b 0a87d47b 66e5dd59 
        129fb717 169bb31b 1a97bf1f 1e93bb03 028fa707 068ba30b 0a87d47b 6aeddd59 
        129fb717 169bb31b 1a97bf1f 1e93bb03 028fa707 068ba30b 0a87d47b 62efc67a */

    int i;

    m_fin = (buffer[0] >> 7 == 0x0) ? DataFrame::Bool::NotSet : DataFrame::Bool::Set;

    if (buffer[0] >> 6 == 0x1)
        m_rsv |= DataFrame::RSV::RSV1;
    if (buffer[0] >> 5 == 0x1)
        m_rsv |= DataFrame::RSV::RSV2;
    if (buffer[0] >> 4 == 0x1)
        m_rsv |= DataFrame::RSV::RSV3;

    m_opcode = (DataFrame::Opcode) (buffer[0] & 0b1111);

    m_mask = (buffer[1] >> 7 == 0x0) ? DataFrame::Bool::NotSet : DataFrame::Bool::Set;


    uint8_t header_end = 2;

    //   The length of the "Payload data", in bytes: if 0-125, that is the
    //   payload length.

    m_payload_len_bytes = buffer[1] & 0b1111111;

    if (m_payload_len_bytes == 126) {

        // If 126, the following 2 bytes interpreted as a
        // 16-bit unsigned integer are the payload length.

        // std::cout << "Extended payload lenght\n";

        m_payload_len_bytes = (uint16_t) buffer[header_end] << 8;
        m_payload_len_bytes += (uint16_t) buffer[header_end+1];

        header_end += 2;

    }

    if (m_payload_len_bytes == 127) {

        // std::cout << "Extended Extended payload lenght\n";

        if ((buffer[header_end] >> 7) == 0b1) {
            std::cout << "\nthe most significant bit MUST be 0\n";
            buffer[header_end] = 0;
        }

        m_payload_len_bytes = 0;

        for (i = 0; i < 8; i++)
            m_payload_len_bytes += (uint64_t) buffer[header_end+i] << (8*(7-i));
        
        header_end += 8;

    }

    if (m_mask == DataFrame::Bool::Set) {
        
        for (i = 0; i < 4; i++)
            m_masking_key[i] = buffer[header_end+i];

        header_end += 4;
    }

    // 26866110 68826514 648e6918 608a6d1c 7c967100 78927504 749e7973 04f6106a 
    // 1dfe1b65 05cd6211 6f836615 6b8f6a19 678b6e1d 26934c27 4dab033f 49a94427 
    // 44e74c35 0caa4620 5fa64436 16e71260 15f51362 1dcd | 81ff 00000000 00153dad 
    // 53d7ee67 1295ad23 1691a92f 1a9da52b 1e99a137 0285bd33 0681b93f 0a8dce4f 
    // 61e1c76d 1295ad23 1691a92f 1a9da52b 1e99a137 0285bd33 0681b93f 0a8dce4f 
    // 66efc76
    return add_payload_data(buffer, header_end, buffer_size);

}

DataFrame DataFrame::get_text_frame(std::string string) {

    DataFrame frame;

    frame.m_fin = DataFrame::Set;
    frame.m_mask = DataFrame::NotSet;
    frame.m_rsv = 0;
    frame.m_opcode = DataFrame::TextFrame;
    frame.m_application_data = std::vector<uint8_t> (string.begin(), string.end());
    frame.m_payload_len_bytes = frame.m_application_data.size();

    return frame;

}

DataFrame DataFrame::get_ping_frame() {

    DataFrame frame;

    frame.m_fin = DataFrame::Set;
    frame.m_mask = DataFrame::NotSet;
    frame.m_rsv = 0;
    frame.m_opcode = DataFrame::Ping;
    frame.m_payload_len_bytes = 0;

    return frame;

}