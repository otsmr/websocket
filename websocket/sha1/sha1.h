/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
typedef u_int8_t uint8_t;
#endif

void sha1 (uint8_t *input, uint8_t *output, int length);