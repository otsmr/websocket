/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "sha1.h"

namespace Hash {

#define S(w, n) (((w) << (n)) | ((w) >> (32-(n))))

void sha1 (uint8_t *input, uint8_t *output, size_t length) {   

    int t;
    int wcount;
    uint32_t A, B, C, D, E;
    uint32_t f;
    uint32_t tmp;
    uint32_t W[80];
    size_t pos = 0;
    uint8_t padding[64+64]{};
    uint8_t padding_length = 64 - (length % 64);

    if (padding_length < 5) { // Padding: "1" + 0's + length (4*8 bits)
        padding_length += 64;
    }

    // 4. Message Padding
    if (padding_length > 0) {

        memset(padding, 0x00, padding_length); // "0"s are appended.
        padding[0] = 0x80; // 1 is appended.
        for (char i = 0; i < 4; i++) // 4-word representation of l
            padding[padding_length-i-1] = (uint8_t) ((length*8) >> (i*8)) & 0xff;

    }

    const uint32_t K[] =    {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };

    uint32_t H[] = {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
    };

    while (pos < (length + padding_length))
    {
        
        memset (W, 0, 80 * sizeof (uint32_t));

        A = H[0];
        B = H[1];
        C = H[2];
        D = H[3];
        E = H[4];

        for (t = 0; t <= 79; t++)
        {
            if (t <= 15) {
                wcount = 24;
                while (wcount >= 0)
                {
                    if (pos < length) {
                        W[t] += ((uint32_t) input[pos]) << wcount;
                    } else {
                        W[t] += ((uint32_t) padding[pos-length]) << wcount;
                    }
                    pos++;
                    wcount -= 8;
                }
            } else {
                W[t] = S(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
            }
            
            unsigned char tt = t/20;
            if (tt == 0)
                f = (B & C) | ((~B) & D);
            else if (tt == 1 || tt == 3)
                f = B ^ C ^ D;
            else if (tt == 2)
                f = (B & C) | (B & D) | (C & D);

            tmp = S(A, 5) + f + E + W[t] + K[tt];
            E = D;
            D = C;
            C = S(B, 30);
            B = A;
            A = tmp;
        }

        H[0] += A;
        H[1] += B;
        H[2] += C;
        H[3] += D;
        H[4] += E;
        
    }

    for(int i = 0; i < 20; ++i)
        *(output+i) = H[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    
}

} // namespace Hash