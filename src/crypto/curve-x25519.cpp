/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "curve-x25519.h"
#include "iostream"
#include "vector"

namespace Crypto {

namespace x25519 {


Point point_multiplication (Point p, uint8_t x[BYTES]) {

    Point result;




    return result;


};

uint8_t modular_multiply(uint8_t a, uint8_t b, uint8_t p) {

    // Compute r = (a * b) mod p

    uint8_t r;

    return r;

};

uint8_t modular_add();

uint32_t get_inverse(uint32_t a, uint32_t p) {

    // Fermat's little theorem
    // r = a^-1 mod p

    uint32_t r;r



    return r;
};

Point point_addition (Point a, Point b) {

    a = {5, 1};
    b = {5, 1};
    

    std::cout << "\n\n2P = P + P = (5,1) + (5,1) = (x, y)\n\n" << std::endl;

    

    Point c;

    uint32_t x = get_inverse(473, 2413);

    std::cout << "get_inverse(473, 2413)" << x << std::endl;

    return c;

    uint8_t s;

    // Parameter der elliptischen Kurve
    const uint8_t E_p = 17U;
    const uint8_t E_a = 2U;

    // 1. s berechnen

    if (a.x == b.x && a.y == b.y) {
        // wenn es sich um den selben Punkt handelt

        // (3x1^2 + a) / 2y1


        // uint8_t oben =  modular_multiply(modular_multiply(a.x, a.x, E_p), 3, E_p) + E_a;
        // std::cout << "oben=" << oben << std::endl;

        uint8_t inverse = get_inverse(2 * a.y, E_p);

        std::cout << "inverse=" << inverse << std::endl;

        // s =  modular_multiply(oben, inverse, E_p);

    } else {

        // a != b
        // s = (y2-y1) / (x2-x1) mod p

    }

    std::cout << "s=" << s << std::endl;
    
    // x3 = s^2 -x1 - x2 mod p
    // y3 = s(x1  x3) - y1 mod p

    return c;


};


} // namespace x25519
} // namespace Crypto