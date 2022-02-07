/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <cstring>
#include <cstdlib>
#include <cmath>

#ifdef __linux__
typedef u_int8_t uint8_t;
#endif

class Base63 {
public:
    // The value of this header field MUST be a
    // nonce consisting of a randomly selected 16-byte value that has
    // been base64-encoded (see Section 4 of [RFC4648]).
    static char [20] encode(const uint8_t [16]);

};

void base64_decode (char *input, uint8_t **output, size_t *length);
void base64_encode (const uint8_t *input, char **output, size_t length);