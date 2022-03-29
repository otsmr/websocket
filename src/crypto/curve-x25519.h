/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <cstdint>

namespace Crypto {

namespace x25519 {


// demo params
static const uint8_t BYTES = 1;

// static const uint8_t BITS = 255;
// static const uint8_t BYTES = 32;
// static const uint8_t WORDS = 8;
// static const uint32_t A24 = 121666;


struct Point {
    // uint8_t x[BYTES];
    // uint8_t y[BYTES];
    uint8_t x;
    uint8_t y;
};

/*
 * Multiplies a point x times on the curve
 *
 * @param[in] Point
 * @param[in] x      
 * @param[out] Point
 * 
 * Returns the multiplied point
 */

Point point_multiplication (Point p, uint8_t x);
// -> double and add

Point point_addition (Point a, Point b);

} // namespace x25519
} // namespace Crypto