/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "base64.h"

char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_decode (char * input, char * output, int length) {

}

void base64_encode (char * input, char * output, int length) {

    char *o = output;

    while (1)
    {

        if (*input == '\00' || *(o-1) == '=')
            break;

        *(o+0) = base64_table[   *input >> 2                                    ];
        *(o+1) = base64_table[ ( *input    & 3   ) << 4 | (*(input+1) >> 4)     ];
        *(o+2) = base64_table[ (*(input+1) & 15  ) << 2 | (*(input+2) >> 6)     ];
        *(o+3) = base64_table[  *(input+2) & 63                                 ];
        
        if (*(input+2) == '\00') {
            *(o+3) = '=';
        }

        if (*(input+1) == '\00') {
            *(o+2) = '=';
            *(o+3) = '=';
        }

        o += 4;
        input += 3;

    }

    *o = '\00';

}