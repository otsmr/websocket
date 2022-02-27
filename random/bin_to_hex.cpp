#include <stdio.h>
#include <ctype.h>
void bin_to_hex (uint8_t * input, char * output, unsigned int len) {
    for (char i = 0; i < len; i++)
        snprintf((output+(i*2)), 4, "%02x", input[i]);
    for (char i = 0; i < (len*2); i++)
        output[i] = toupper(output[i]);
    output[len*2] = 0b0;
}