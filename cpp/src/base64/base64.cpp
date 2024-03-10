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

void decode (const char *input, uint8_t *output, size_t *out_len) {

    int len = strlen(input);

    if (len % 4 != 0)
        return;

    *out_len = len - len/4;

    char i;
    uint8_t tmp[4];
    uint8_t *out = output;

    char reverse_characters['z'+1];
    for (i = 0; i < 65; i++)
        reverse_characters[characters[i]] = i;

    while (*input != '\00')
    {
        for (i = 0; i < 4; i++)
        {
            if (*(input+i) == '=') {
                *(tmp+i) = '\00';
                (*out_len)--;
            } else {
                *(tmp+i) = reverse_characters[*(input+i)];
            }
        }
    
        *(out+0) = (*(tmp+0) << 2) | (*(tmp+1) >> 4);
        *(out+1) = (*(tmp+1) << 4) | (*(tmp+2) >> 2);
        *(out+2) = ((*(tmp+2) & 15) << 6) | *(tmp+3);
        
        input  += 4;
        out    += 3;

    }
    
}

} // namespace Base64