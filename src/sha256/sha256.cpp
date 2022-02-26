/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "sha256.h"

namespace Hash {

#define ROTR(n,w) ((w >> n) | ( (w << ((32-n) & 31)) & 0xffffffff ))

#define CH(x, y, z) ((x & y) ^ ((~x) & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define BSIG0(x) (ROTR(2, x) ^ ROTR(13, x) ^ ROTR(22, x))
#define BSIG1(x) (ROTR(6, x) ^ ROTR(11, x) ^ ROTR(25, x))
#define SSIG0(x) (ROTR(7, x) ^ ROTR(18, x) ^ (x >> 3))
#define SSIG1(x) (ROTR(17, x) ^ ROTR(19, x) ^ (x >> 10))

void sha256 (uint8_t *input, uint8_t *output, unsigned int length) {   

    int t;
    int wcount;
    uint8_t *message = nullptr;
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t T1, T2;
    uint32_t pos = 0;
    uint32_t W[64];
    uint64_t len_pad = ((length) + (64 - (length%64))); // length + padding in bytes

    if ((length % 64) != 0 && ((length*8)+65) > 512) {
        len_pad += 64;
    }

    message = (uint8_t *) calloc(len_pad+2, sizeof(uint8_t));
    memcpy(message, input, length);

    // 4. Message Padding
    if ((length % 64) != 0 || length == 0) {

        // a. "1" is appended.
        message[length] = 0x80;
        // b. "0"s are appended. -> calloc()
        // c. 4-word representation of l
        for (char i = 0; i < 16; i++)
            *((message + len_pad) - i - 1) = (uint8_t) ((length*8) >> (i*8)) & 0xff;

    }

    const uint32_t K[] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
      0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
      0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
      0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
      0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
      0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t H[] = {
      0x6a09e667,
      0xbb67ae85,
      0x3c6ef372,
      0xa54ff53a,
      0x510e527f,
      0x9b05688c,
      0x1f83d9ab,
      0x5be0cd19
    };

    for (int i = 0; i < (len_pad / 64); i++)
    {
        
        memset (W, 0, 64 * sizeof (uint32_t));

        a = H[0];
        b = H[1];
        c = H[2];
        d = H[3];
        e = H[4];
        f = H[5];
        g = H[6];
        h = H[7];

        for (t = 0; t <= 63; t++)
        {

            if (t <= 15) {
                wcount = 24;
                while (wcount >= 0)
                {
                    W[t] += (((uint32_t) message[pos]) << wcount);
                    pos++;
                    wcount -= 8;
                }
            } else {
                W[t] = SSIG1(W[t-2]) + W[t-7] + SSIG0(W[t-15]) + W[t-16];
            }

            T1 = h + BSIG1(e) + CH(e, f, g) + K[t] + W[t];
            T2 = BSIG0(a) + MAJ(a, b, c);

            printf("t[%d] T1[%u] T2[%u]\n", t, T1, T2);

            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
        
    }

    for (int i = 0;i<8;i++)
        printf("H[%d]=%u\n", i, H[i]);

    for(int i = 0; i < 32; ++i)
        output[i] = H[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    
}

} // namespace Hash