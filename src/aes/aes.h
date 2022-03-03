/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <cstdint>
#include <string.h>

typedef uint8_t u8;

class AES {
public:

    enum Blocksize {
        b128,
        b192,
        b256
    };

    AES() = default;
    AES(Blocksize blocksize) { m_blocksize = blocksize; };
    ~AES() = default;

    /*
     * AES sequence encrypt
     * The input and output each consist of sequences of 128 bits.
     */
    bool encrypt(uint8_t *input, uint8_t * output, uint8_t key[32]);
    bool decrypt(uint8_t *input, uint8_t * output, uint8_t key[32]);

private:
    Blocksize m_blocksize { b256 };

    uint8_t m_state[16];
    u8 m_key[32];

    void AddRoundKey();

    void InvMixColumns();
    void MixColumns();
    void ShiftRows();
    void InvShiftRows();
    void InvSubBytes();
    void SubBytes();

    void RotWord();
    void SubWord();

};