/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include <string>
#include "base64.h"

int main (int argc, char** argv) {

    char *input;
    char *output;

    char test_str[] = "Hallo Welt!";

    int len = strlen(test_str);
    
    input = (char *) calloc(len, sizeof(char));
    output = (char *) calloc(4 * (len / 3), sizeof(char));

    strncpy(input, test_str, len);

    base64_encode(input, output, len);

    printf("%s", output);

    return 0;

}