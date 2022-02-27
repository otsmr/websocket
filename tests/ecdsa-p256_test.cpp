#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tls13/ecdsa-p256.h"
#include "base64/base64.h"
#include "hash/sha256.h"

void bin_to_hex (uint8_t * input, char * output, unsigned int len) {
    for (char i = 0; i < len; i++)
        snprintf((output+(i*2)), 4, "%02x", input[i]);
    for (char i = 0; i < (len*2); i++)
        output[i] = toupper(output[i]);
    output[len*2] = 0b0;
}

void test_ecdsa_p256() {

    printf("\n -- Example of ECDSA with P-256 -- \n\n");


    char message[] = "Example of ECDSA with P-256";
    uint8_t hash[32]{};
    Hash::sha256((uint8_t *) message, hash, strlen(message));


    char b64_private_key[] = "xHf59lwizOIGV/qlstHYEiM2+FGlCKHtBOR5w0mFv5Y=";
    uint8_t private_key[32+1]{};
    size_t o = -1;

    Base64::decode(b64_private_key, private_key, &o);

    char D[64+1]{};
    bin_to_hex(private_key, D, 32);
    printf("D: %s\n", D);

    ECDSA::Signature signature;
    ECDSA::sign(private_key, hash, &signature);

    char S[64+1]{};
    char R[64+1]{};
    bin_to_hex(signature.r, R, 32);
    bin_to_hex(signature.s, S, 32);

    printf("-----\nSignature\nR: %s\n", R);
    printf("S: %s\n", S);

    printf("\nFAILED");
}

int main(int argc, char **argv) {

    test_ecdsa_p256();

}