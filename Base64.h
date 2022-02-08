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

namespace Base64 {
    /*
     * Covert byte values to the Base64 representation.
     *
     * @param[in] input 20-byte-value
     * @param[out] output 29-length char array. \00 terminated.
     *              ceil((20/3)*4) = 28 + len(\00) = 29
     */
    void encode(const uint8_t [20], char *output);
}; // namespace Base64