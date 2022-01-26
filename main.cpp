/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include <string>
#include "base64.h"

int main (int argc, char** argv) {

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

    return 0;

}