/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "crypto/aes-gcm.h"
#include "hash/sha256.h"
#include "hash/sha384.h"
#include "crypto/curve-x25519.h"

namespace TLS {


class TLSv13  {

public:

    uint8_t read(uint8_t * buffer, size_t size);
    uint8_t write(uint8_t * buffer, size_t size);


private:


};




} // namespace TLS