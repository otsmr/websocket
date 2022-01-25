/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "base64.h"

char characters[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_decode (char * input, char * output, int length) {

}

void base64_encode (char * input, char **output, int length) {

    *output = (char *) calloc(ceil(length * (4/3) + 1), sizeof(char));

    char * out = *output;
    char tmp[3];
    int pos = 0;
    
    while ((length-pos) > 0)
    {

        for (char i = 0; i < 3; i++)
        {
            if ((length-pos-i) <= 0) {
                *(tmp+i) = '\00';
            } else {
                *(tmp+i) = *(input+pos+i);
            }
        }

        *(out+0) = characters[   *tmp >> 2                              ];
        *(out+1) = characters[ ( *tmp    & 3   ) << 4 | (*(tmp+1) >> 4) ];
        *(out+2) = characters[ (*(tmp+1) & 15  ) << 2 | (*(tmp+2) >> 6) ];
        *(out+3) = characters[  *(tmp+2) & 63                           ];
    
        out += 4;
        pos += 3;

    }

    for (char i = 0; i < (3 - length%3)%3; i++)
    {
        *(out-i-1) = '=';
    }

    *out = '\00';

}