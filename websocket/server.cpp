/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <string>
#include "base64/base64.h"
#include "sha1/sha1.h"

int main (int argc, char** argv) {

    uint8_t hash[20];
    char * output = NULL;

    char string[] = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1((uint8_t *) string, hash, strlen(string));

    base64_encode(hash, &output, 20);

    printf("IST:  %s\n", output);
    printf("SOLL: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\n");

    return 0;

}