/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <string.h>
#include <stdlib.h>
#include <cmath>

void base64_decode (char *input, uint8_t **output, uint64_t *length);
void base64_encode (uint8_t *input, char **output, uint64_t length);