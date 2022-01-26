/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <string.h>
#include <stdlib.h>
#include <cmath>

void base64_decode (char *input, char **output, int *output_length);
void base64_encode (char *input, char **output, int length);