/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <cstdint>
#include <string.h>

namespace Crypto {

typedef uint8_t byte;

class AES {
public:

    enum Blocksize {
        b128 = 128,
        b192 = 192,
        b256 = 256
    };

    AES() = default;
    AES(Blocksize blocksize) { m_blocksize = blocksize; };
    ~AES() = default;

    /*
     * AES sequence encrypt
     * The input and output each consist of sequences of 128 bits.
     */
    bool encrypt(uint8_t *input, uint8_t * output);
    bool decrypt(uint8_t *input, uint8_t * output);

    void update_key(byte * key);

private:
    Blocksize m_blocksize { b256 };

    uint8_t m_state[16];
    byte m_expanded_key[60][4];

    void SubWord(byte * word);
    void RotWord(byte * word);

    void SubBytes();
    void InvSubBytes();
    void ShiftRows();
    void InvShiftRows();
    void MixColumns(bool inverse);

    void AddRoundKey(byte round);
    void KeySchedule();

};

}