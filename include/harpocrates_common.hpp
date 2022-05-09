#pragma once
#include <cstdint>

// Harpocrates - An Efficient Encryption Mechanism for Data-at-rest, related
// common functions & constants
namespace harpocartes_utils {

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
constexpr uint16_t RC2[8] = { 32, 8, 2, 32768, 8192, 2048, 512, 128 };

// Harpocrates round-3 constants, to be added to state matrix during execution
// of round-3, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC3[8] = { 2048, 512, 128, 32, 8, 2, 32768, 8192 };

// Harpocrates round-4 constants, to be added to state matrix during execution
// of round-4, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC4[8] = { 8, 2, 32768, 8192, 2048, 512, 128, 32 };

// Harpocrates round-5 constants, to be added to state matrix during execution
// of round-5, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC5[8] = { 8192, 2048, 512, 128, 32, 8, 2, 32768 };

// Harpocrates round-6 constants, to be added to state matrix during execution
// of round-6, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC6[8] = { 512, 128, 32, 8, 2, 32768, 8192, 2048 };

// Harpocrates round-7 constants, to be added to state matrix during execution
// of round-7, generated following `Round constant addition` point in
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
constexpr uint16_t RC7[8] = { 128, 32, 8, 2, 32768, 8192, 2048, 512 };

}
