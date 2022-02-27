/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "ecdsa-p256.h"
#include <stdio.h>
#include "../base64/base64.h"

namespace ECDSA {

#define DEV 1

#if DEV
#include <stdio.h>
#include <ctype.h>
void bin_to_hex (uint8_t * input, char * output, unsigned int len) {
    for (char i = 0; i < len; i++)
        snprintf((output+(i*2)), 4, "%02x", input[i]);
    for (char i = 0; i < (len*2); i++)
        output[i] = toupper(output[i]);
    output[len*2] = 0b0;
}
#endif


bool get_random_number (uint64_t * random_number) {

    FILE *fd = fopen("/dev/random", "r");
    if (fd == NULL)
        return false;

    char * ptr = (char*) random_number;

    for (int i = 0; i < ECC_BYTES; i++)
    {
        uint8_t c = fgetc(fd);
        *(ptr+i) += c;
    }
    fclose(fd);
    return true;

}

bool sign( uint8_t private_key[ECC_BYTES], uint8_t hash[ECC_BYTES], Signature * signature ) {


// --- TEST Start ---
#if DEV
    char H[64+1]{};
    bin_to_hex(hash, H, 32);
    printf("H: %s\n", H);
#endif
// --- TEST End ---


    uint64_t random_number[ECC_DIGITS];

    // TODO: random_number_generator

// --- TEST Start ---
#if DEV
    char b64_random_number[] = "ehp+Unl/yMqqQ10qTazjkVhQS/IE++GfFNu0J/ruUK4=";
    size_t o = -1;

    Base64::decode(b64_random_number, (uint8_t *) random_number, &o);

    char K[64+1]{};
    bin_to_hex((uint8_t *) random_number, K, 32);
    printf("K: %s\n", K);

#else

    if (!get_random_number(random_number))
        return false;

#endif

    printf("RANDOM: ");
    for (size_t i = 0; i < ECC_DIGITS; i++)
        printf("%llu", random_number[i]);
    printf("\n");


// --- TEST End ---


    /*
     * Compute the Elliptic Curve point 𝑅=𝑘𝐺 of the curve P-256,
     * where 𝐺 is the generator point. 𝑅 has Cartesian coordinates
     * (𝑥𝑅,𝑦𝑅) (the question's R_x and R_y), but only 𝑥𝑅 is needed.
    */



    // char R_x[64+1]{};
    // bin_to_hex(R.x, R_x, 32);
    // printf("R_x: %s\n", R_x);
    // char R_y[64+1]{};
    // bin_to_hex(R.y, R_y, 32);
    // printf("R_y: %s\n", R_y);


    return signature;

};

bool verify( uint8_t public_key[ECC_BYTES], uint8_t hash[ECC_BYTES], Signature signature ) {
    return false;
}
} // namespace ECDSA