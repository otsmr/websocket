/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <cstdint>

namespace ECDSA {

// secp256r1
#define ECC_BYTES 32
#define ECC_DIGITS (ECC_BYTES/8)

typedef struct EccPoint
{
    uint64_t x[ECC_DIGITS];
    uint64_t y[ECC_DIGITS];
} EccPoint;

struct Signature {
    uint8_t r[ECC_BYTES];
    uint8_t s[ECC_BYTES];
};

bool create_key ();
bool shared_secret ();

/*
 * Generate an ECDSA signature for a given hash value.
 *
 * @param[in] private_key
 * @param[in] hash
 * @param[out] signature
 * 
 * Returns true if it was successfully, false if an error occurred.
 */
bool sign(
    uint8_t private_key[ECC_BYTES],
    uint8_t hash[ECC_BYTES],
    Signature * signature
);

/*
 * Verify an ECDSA signature.
 *
 * @param[in] public_key
 * @param[in] hash
 * @param[in] signature
 * 
 * Returns true if the signature is valid, false if it is invalid.
 */
bool verify(
    uint8_t public_key[ECC_BYTES],
    uint8_t hash[ECC_BYTES],
    Signature signature
);

} // namespace ECDSA