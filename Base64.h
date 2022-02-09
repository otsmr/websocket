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
     * @param[in] input char array
     * @param[in] len of the input char array
     * @param[out] output char array. \00 terminated.
     *              ceil((len/3)*4) + len(\00)
     */
    void encode(const uint8_t *input, char *output, size_t len);
}; // namespace Base64