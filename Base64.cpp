/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "Base64.h"

char characters[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * Base64::encode(const uint8_t input[20]) {

    static char out[29];
    int pos, j;

    uint8_t tmp[3];

    for (pos = 0; pos < 20; pos+=3) {

        for (j = 0; j < 3; j++)
        {
            if ((28-pos-j) <= 0) {
                tmp[j] = '\00';
            } else {
                tmp[j] = input[pos+j];
            }
        }

        out[(pos/3)*4+0] = characters[   *tmp >> 2                                  & 63 ];
        out[(pos/3)*4+1] = characters[ (( *tmp    & 3   ) << 4 | (*(tmp+1) >> 4))   & 63 ];
        out[(pos/3)*4+2] = characters[ ((*(tmp+1) & 15  ) << 2 | (*(tmp+2) >> 6))   & 63 ];
        out[(pos/3)*4+3] = characters[  *(tmp+2)                                    & 63 ];

    }

    out[27] = '=';
    out[28] = '\00';

    return out;

}
