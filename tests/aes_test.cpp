/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "crypto/aes.h"

using namespace Crypto;

void test_aes(AES::Blocksize bs, byte *input, byte *key, byte *expected) {

    AES aes(bs);

    aes.update_key(key);

    byte out_encrypt[16]{};
    byte out_decrypt[16]{};

    if (!aes.encrypt(input, out_encrypt))
        printf("FAILED! !aes.encrypt\n");

    for (int i = 0; i < 16; i++)
        if (expected[i] != out_encrypt[i]) {
            printf("FAILED! expected[i] != out_encrypt[i]\n");
            break;
        }

    if (!aes.decrypt(out_encrypt, out_decrypt))
        printf("FAILED! !aes.decrypt\n");

    for (int i = 0; i < 16; i++)
        if (input[i] != out_decrypt[i]) {
            printf("FAILED! input[i] != out_decrypt[i]\n");
            break;
        }
    
}

int main(int argc, char **argv) {

    byte plaintext[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

    byte key128[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    byte cypher128[] = {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
    test_aes(AES::b128, plaintext, key128, cypher128);

    byte key192[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    byte cypher192[] = {0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0, 0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91};
    test_aes(AES::b192, plaintext, key192, cypher192);

    byte key256[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
    byte cypher256[] = {0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf, 0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89};
    test_aes(AES::b256, plaintext, key256, cypher256);

}