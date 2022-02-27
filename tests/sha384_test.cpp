#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "hash/sha384.h"

void test_sha384(char *string, char expected[97]) {

    uint8_t hash[48]{};
    char hexdigest[97]{};

    Hash::sha384((uint8_t *) string, hash, strlen(string));

    for (char i = 0; i < 48; i++)
        snprintf((hexdigest+(i*2)), 4, "%02x", hash[i]);
    
    for (int i = 0; i <= 96; i++)
    {
        if (expected[i] != hexdigest[i]) {
            printf("\n=%s=\n=%s=\n", expected, hexdigest);
            printf("FAILED");
            return;
        }
    }

}

int main() {

    test_sha384((char *) "", (char *) "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
    test_sha384((char *) "The quick brown fox jumps over the lazy dog", (char *) "ca737f1014a48f4c0b6dd43cb177b0afd9e5169367544c494011e3317dbf9a509cb1e5dc1e85a941bbee3d7f2afbc9b1");
    test_sha384((char *) "The quick brown fox jumps over the lazy cog", (char *) "098cea620b0978caa5f0befba6ddcf22764bea977e1c70b3483edfdf1de25f4b40d6cea3cadf00f809d422feb1f0161b");

}