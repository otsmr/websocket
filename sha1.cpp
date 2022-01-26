/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "sha1.h"

void p(char *input, int length) {
    printf("\nMessage: \n");
    for (int i = 0; i < length; i++)
    {
        printf("%x", (*(input+i) >> 4) & 0b00001111);
        printf("%x", *(input+i) & 0b1111);
        if ((i+1) % 4 == 0) {
            printf(" ");
        }
        if ((i+1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void sha1 (char *input, char *output, int length) {

    /*
    * 4. Message Padding
    * 
    * The purpose of message padding is to make the total length of a
    * padded message a multiple of 512.
    * 
    * As a summary, a "1" followed by m "0"s followed by a 64-
    * bit integer are appended to the end of the message to produce a
    * padded message of length 512 * n.  The 64-bit integer is the length
    * of the original message.
    * 
    */

    // The length of the message is the number of bits in the message 
    // (the empty message has length 0). The 64-bit integer is the length
    // of the original message.

    unsigned long long l = length * 8;

    char *message = NULL;

    int len_pad = ((length) + (64 - (length%64)));

    if ((l % 512) != 0 && (l+65) > 512) {
        len_pad += 64;
    }

    message = (char *) calloc(len_pad, sizeof(char));
    memcpy(message, input, length);

    if ((l % 512) != 0) {

        p(message, len_pad);

        // a. "1" is appended.
        // adding 0x80
        *(message+length) = 0x80;
        p(message, len_pad);

        // b. "0"s are appended. 
        //    "0" * (length % 512) - 1 (a.) - 64 (c.)
        // -> calloc()

        // c. 2-word representation of l
        //    If l < 2^32 then the first word is all zeroes. Append these
        //    two words to the padded message.

        for (int i = 0; i < 8; i++)
        {
            *(message + len_pad - i - 1) = (l >> (i*8)) & 0b11111111;
        }
        
        p(message, len_pad);

    }


}