

#include <stdlib.h>

#ifdef __linux__
typedef u_int8_t uint8_t;
#endif

void test_parse_data_frame (uint8_t * data) {

    // DataFrame frame = WebSocket::parse_data_frame(data);

    // printf("Datenlaenge (%llu)\n", frame.payload_len_bytes);
    // printf("MASK (%d)\n", frame.mask);
    // printf("FIN (%d)\n", frame.fin);
    // printf("Opcode (%d)\n", frame.fin);

    // switch (frame.opcode)
    // {
    // case DataFrame::Opcode::TextFrame:
    //     printf("-- Text --\n");

    //     std::cout << frame.text_data << "\n";

    //     break;
    
    // case DataFrame::Opcode::BinaryFrame:
    //     printf("-- Binary --\n");
    //     break;
    // default:
    //     break;
    // }

}

int main() {

}
    /*
    // A single-frame unmasked text message
    uint8_t single_frame_unmasked_text [] = {
        0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f
    }; // (contains "Hello")

    // test_parse_data_frame(single_frame_unmasked_text);
    // return 0;

    // A single-frame masked text message
    uint8_t single_frame_masked_text [] = {
        0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
    }; // (contains "Hello")

    test_parse_data_frame(single_frame_masked_text);
    return 0;

    // A fragmented unmasked text message
    uint8_t fragmented_unmasked_text [2][6] = {
        {0x01, 0x03, 0x48, 0x65, 0x6c}, // (contains "Hel")
        {0x80, 0x02, 0x6c, 0x6f} // (contains "lo")
    }; // (contains "Hello")

    // Unmasked Ping request and masked Ping response
    uint8_t unmasked_ping [] = {
        0x89, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f
    }; // (contains a body of "Hello", but the contents of the body are arbitrary)

    uint8_t masked_ping [] = {
        0x8a, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58
    }; // (contains a body of "Hello", matching the body of the ping)

    */

//    o  256 bytes binary message in a single unmasked frame
//       *  0x82 0x7E 0x0100 [256 bytes of binary data]

//    o  64KiB binary message in a single unmasked frame
//       *  0x82 0x7F 0x0000000000010000 [65536 bytes of binary data]
