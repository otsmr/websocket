/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "base64.h"

char characters[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

namespace Base64 {

void encode(const uint8_t * input , char * output, size_t len) {

    char * out = output;
    char i;

    uint8_t tmp[3];
    size_t pos = 0;

    while (len > pos)
    {

        for (i = 0; i < 3; i++)
        {
            if ((len-pos-i) <= 0) {
                *(tmp+i) = '\00';
            } else {
                *(tmp+i) = *(input+pos+i);
            }
        }

        *(out+0) = characters[   *tmp >> 2                                  & 63 ];
        *(out+1) = characters[ (( *tmp    & 3   ) << 4 | (*(tmp+1) >> 4))   & 63 ];
        *(out+2) = characters[ ((*(tmp+1) & 15  ) << 2 | (*(tmp+2) >> 6))   & 63 ];
        *(out+3) = characters[  *(tmp+2)                                    & 63 ];
    
        out += 4;
        pos += 3;

    }

    for (i = 0; i < (char) ( 3 - (len%3) ) % 3; i++)
        *(out-i-1) = '=';

    *out = '\00';

}

} // namespace Base64