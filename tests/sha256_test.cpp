/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "hash/sha256.h"

void test_sha256(char *string, char expected[65]) {

    uint8_t hash[32]{};
    char hexdigest[65]{};

    Hash::sha256((uint8_t *) string, hash, strlen(string));

    for (char i = 0; i < 32; i++)
        snprintf((hexdigest+(i*2)), 4, "%02x", hash[i]);
    
    for (int i = 0; i <= 64; i++)
    {
        if (expected[i] != hexdigest[i]) {
            printf("\n=%s=\n=%s=\n", expected, hexdigest);
            printf("FAILED");
            return;
        }
    }

}

int main() {

    test_sha256((char *) "", (char *) "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    test_sha256((char *) "The quick brown fox jumps over the lazy dog", (char *) "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
    test_sha256((char *) "The quick brown fox jumps over the lazy cog", (char *) "e4c4d8f3bf76b692de791a173e05321150f7a345b46484fe427f6acc7ecc81be");

}