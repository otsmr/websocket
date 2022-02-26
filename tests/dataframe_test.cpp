#include <stdlib.h>

#include "websocket/frame.h"

#ifdef __linux__
typedef u_int8_t uint8_t;
#endif

struct Expected {
    bool fin;
    bool mask;
    uint8_t rsv;
    DataFrame::Opcode opcode;
    uint64_t payload_len_bytes;
    uint8_t masking_key[4];
    std::string content;
};

void test_parse_data_frame (uint8_t * data, int len, Expected expected) {

    DataFrame frame;
    frame.parse_raw_frame(data, len);

    if (frame.m_fin != expected.fin)
        printf("FAILED fin");

    if (frame.m_mask != expected.mask)
        printf("FAILED mask");
    
    if (frame.m_rsv != expected.rsv)
        printf("FAILED rsv");

    if (frame.m_opcode != expected.opcode)
        printf("FAILED opcode %x", frame.m_opcode);

    if (frame.m_payload_len_bytes != expected.payload_len_bytes)
        printf("FAILED payload_len_bytes");

    if (frame.m_mask) {
        for (int i = 0; i < 4; i++)
            if (frame.m_masking_key[i] != expected.masking_key[i])
                printf("FAILED masking_key");   
    }

    if (frame.m_opcode == DataFrame::TextFrame &&
        frame.get_utf8_string() != expected.content)
    {
        printf("FAILED get_utf8_string");
    }

}

int main() {
    

    // A single-frame unmasked text message
    uint8_t single_frame_unmasked_text [] = {
        0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f
    }; // (contains "Hello")

    test_parse_data_frame(single_frame_unmasked_text, 7, Expected {
        true, false, 0, DataFrame::TextFrame, 5,
        {0, 0, 0, 0},
        "Hello"
    });


    // A single-frame masked text message
    uint8_t single_frame_masked_text [] = {
        0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
    }; // (contains "Hello")

    test_parse_data_frame(single_frame_masked_text, 11, Expected {
        true, true, 0, DataFrame::TextFrame, 5, 
        {0x37, 0xfa, 0x21, 0x3d},
        "Hello"
    });


    // Unmasked Ping request and masked Pong response
    uint8_t unmasked_ping [] = {
        0x89, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f
    }; // (contains a body of "Hello", but the contents of the body are arbitrary)

    test_parse_data_frame(unmasked_ping, 7, Expected {
        true, false, 0, DataFrame::Ping, 5, 
        {0,0,0,0},
        "Hello"
    });


    uint8_t masked_pong [] = {
        0x8a, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
    }; // (contains a body of "Hello", matching the body of the ping)

    test_parse_data_frame(masked_pong, 11, Expected {
        true, true, 0, DataFrame::Pong, 5, 
        {0x37, 0xfa, 0x21, 0x3d},
        "Hello"
    });

}