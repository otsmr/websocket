/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include <string>
#include "base64.h"
#include "sha1.h"

#define l 5
void test_sha1 () {

    char hash[20];


    char string[l];
    for (int i = 0; i < l; i++)
    {
        string[i] = 0x41;
    }
    
    sha1(string, hash, l);

    // char string[] = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // sha1(string, hash, strlen(string));

    // char *output = NULL;
    // base64_encode(hash, &output, 20);

    // printf("IST: %s\n", output);
    // printf("SOLL: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\n");

}

void test_base64 () {

    char *output = NULL;
    char *string_decoded = NULL;
    int decoded_length = -1;

    char string[] = "TTT\00\00\00TTT";
    base64_encode(string, &output, 9);

    printf("Base64: %s\n", output);

    base64_decode(output, &string_decoded, &decoded_length);

    printf("String (%d): ", decoded_length);
    for (char i = 0; i < decoded_length; i++)
    {
        printf("%x ", *(string_decoded+i));
    }
    
    printf("\n");

}

int main (int argc, char** argv) {

    // test_base64();
    test_sha1();

    return 0;

}