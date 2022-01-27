#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "../websocket/sha1/sha1.h"

void test_sha1(char *string, char expected[41]) {

    uint8_t hash[20];
    char hexdigest[41];

    sha1((uint8_t *) string, hash, strlen(string));

    for (char i = 0; i < 20; i++)
        snprintf((hexdigest+(i*2)), 4, "%02x", hash[i]);
    
    for (int i = 0; i <= 40; i++)
    {
        if (expected[i] != hexdigest[i]) {
            printf("\n=%s=\n=%s=\n", expected, hexdigest);
            printf("FAILED");
            return;
        }
    }

}

int main() {

    test_sha1((char *) "The quick brown fox jumps over the lazy dog", (char *) "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");
    test_sha1((char *) "The quick brown fox jumps over the lazy cog", (char *) "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");
    test_sha1((char *) "", (char *) "da39a3ee5e6b4b0d3255bfef95601890afd80709");

}