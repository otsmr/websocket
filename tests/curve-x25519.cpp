#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "crypto/curve-x25519.h"

using namespace Crypto::x25519;


void test_x25519() {

    Point A = {5, 1};

    point_addition(A, A);

    printf("\nFAILED");
    return;


    Point P = {10, 6};
    uint8_t a = {3};
    
    Point result = point_multiplication(P, a);

    if (
        result.x != 10 &&
        result.y != 6)
    {

        printf("x = %d (10)\ny = %d (6)", result.x, result.y);
        printf("\nFAILED");
    }

}

int main(int argc, char **argv) {

    test_x25519();
    return 0;

}