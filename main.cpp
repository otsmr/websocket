/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include <string>
#include "base64.h"

int main (int argc, char** argv) {

    char *output = NULL;

    char string [] = "1234";
    base64_encode(string, &output, strlen(string));

    printf("%s", output);

    return 0;

}