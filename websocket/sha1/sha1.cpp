/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "sha1.h"
#include <cstdint>

#define S(word, bits) \
            (((word) << (bits)) | ((word) >> (32-(bits))))

void sha1 (uint8_t *input, uint8_t *output, int length) {   

    int t;
    int wcount;
    uint8_t *message = nullptr;
    uint32_t A, B, C, D, E;
    uint32_t f;
    uint32_t tmp;
    uint32_t pos = 0;
    uint32_t W[80];
    size_t l = length * 8;
    size_t len_pad = ((length) + (64 - (length%64)));


    if ((l % 512) != 0 && (l+65) > 512) {
        len_pad += 64;
    }

    message = (uint8_t *) calloc(len_pad+2, sizeof(uint8_t));
    memcpy(message, input, length);

    // 4. Message Padding

    if ((l % 512) != 0 || l == 0) {

        // a. "1" is appended.
        *(message+length) = 0x80;
        // b. "0"s are appended. -> calloc()
        // c. 2-word representation of l
        for (char i = 0; i < 8; i++)
            *((message + len_pad) - i-1) = (uint8_t) (l >> (i*8)) & 0xff;

    }

    // 5. Functions and Constants Used

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

    // 6. Computing the Message Digest (6.1 Method 1)

    for (int i = 0; i < (len_pad / 64); i++)
    {
        
        // a. Divide M(i) into 16 words W(0), W(1), ... , W(15), where W(0)
        //    is the left-most word.
        memset (W, 0, 80 * sizeof (uint32_t));

        for (t = 0; t <= 15; t++)
        {
            wcount = 24;
            while (wcount >= 0)
            {
                W[t] += (((uint32_t) *(message+pos)) << wcount);
                pos++;
                wcount -= 8;
            }
        }
        
        // b. For t = 16 to 79 let
        for (t = 16; t <= 79; t++)
            W[t] = S(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);

        // c. Let A = H0, B = H1, C = H2, D = H3, E = H4.
        A = H[0];
        B = H[1];
        C = H[2];
        D = H[3];
        E = H[4];

        // d. For t = 0 to 79 do
        for (t = 0; t <= 79; t++)
        {
            // 5. Functions and Constants Used
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

        // e. Let H0 = H0 + A, H1 = H1 + B, H2 = H2 + C, H3 = H3 + D, H4 = H4+ E.

        H[0] += A;
        H[1] += B;
        H[2] += C;
        H[3] += D;
        H[4] += E;
        
    }

    for(int i = 0; i < 20; ++i)
        *(output+i) = H[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    
}