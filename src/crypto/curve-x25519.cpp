/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "curve-x25519.h"

namespace Crypto {

namespace x25519 {


Point point_multiplication (Point p, uint8_t x[BYTES]) {

    Point result;




    return result;


};

// modular_multiply();

Point point_addition (Point a, Point b) {

    uint8_t s;
    uint8_t p = 17;

    // s Berechnen
    if (a.x == b.x && a.y == b.y) {
        // a == b
        // (3x1^2 + a) / 2y1       -> a = 2


        uint8_t oben = (a.x * a.x * 3) + 2; // modular_multiply
        uint8_t unten = 2 * a.y;            // inverse finden

        s = (oben * unten) % p;

    } else {

        // a != b
        // s = (y2-y1) / (x2-x1) mod p

    }





    // x3 = s^2 -x1 - x2 mod p
    // y3 = s(x1  x3) - y1 mod p

    // berechnen einer Inversen
};


} // namespace x25519
} // namespace Crypto