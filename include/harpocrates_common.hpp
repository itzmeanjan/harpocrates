#pragma once
#include <cstdint>

using size_t = std::size_t;

// Harpocrates - An Efficient Encryption Mechanism for Data-at-rest, related
// common functions & constants
namespace harpocrates_common {

// 16 -bytes of plain/ cipher text blocks are processed at a time
// by Harpocrates cipher ( which is why input to encrypt()/ decrypt() routines
// are expected to have 16 -bytes of plain/ cipher text )
constexpr size_t BLOCK_LEN = 16ul;

// Harpocrates cipher consists of 8 iterative rounds for processing
// each message block ( 16 -bytes )
constexpr size_t N_ROUNDS = 8ul;

// # -of rows in Harpocrates state matrix ( it's a 8 x 16 row-major bit matrix )
constexpr size_t N_ROWS = BLOCK_LEN >> 1;

// # -of columns in Harpocrates state matrix ( it's a 8 x 16 row-major bit
// matrix )
constexpr size_t N_COLS = 16ul;

// 16 -bit masks for selecting ( read enabling/ disabling ) column(s), when
// performing column substitution
constexpr uint16_t COL_MASKS[16] = { 0b0111111111111111, 0b1011111111111111,
                                     0b1101111111111111, 0b1110111111111111,
                                     0b1111011111111111, 0b1111101111111111,
                                     0b1111110111111111, 0b1111111011111111,
                                     0b1111111101111111, 0b1111111110111111,
                                     0b1111111111011111, 0b1111111111101111,
                                     0b1111111111110111, 0b1111111111111011,
                                     0b1111111111111101, 0b1111111111111110 };

// Harpocrates round-0 constants, to be added to state matrix during execution
// of round-0, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC0[8] = { 32768, 8192, 2048, 512, 128, 32, 8, 2 };

// Harpocrates round-1 constants, to be added to state matrix during execution
// of round-1, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC1[8] = { 2, 32768, 8192, 2048, 512, 128, 32, 8 };

// Harpocrates round-2 constants, to be added to state matrix during execution
// of round-2, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC2[8] = { 8, 2, 32768, 8192, 2048, 512, 128, 32 };

// Harpocrates round-3 constants, to be added to state matrix during execution
// of round-3, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC3[8] = { 32, 8, 2, 32768, 8192, 2048, 512, 128 };

// Harpocrates round-4 constants, to be added to state matrix during execution
// of round-4, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC4[8] = { 128, 32, 8, 2, 32768, 8192, 2048, 512 };

// Harpocrates round-5 constants, to be added to state matrix during execution
// of round-5, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC5[8] = { 512, 128, 32, 8, 2, 32768, 8192, 2048 };

// Harpocrates round-6 constants, to be added to state matrix during execution
// of round-6, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC6[8] = { 2048, 512, 128, 32, 8, 2, 32768, 8192 };

// Harpocrates round-7 constants, to be added to state matrix during execution
// of round-7, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC7[8] = { 8192, 2048, 512, 128, 32, 8, 2, 32768 };

}
