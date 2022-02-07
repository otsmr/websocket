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

class Base64 {
public:
    // The value of this header field MUST be a
    // nonce consisting of a randomly selected 16-byte value that has
    // been base64-encoded (see Section 4 of [RFC4648]).
    static char * encode(const uint8_t [20]);
};