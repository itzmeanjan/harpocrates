#pragma once
#include "harpocrates_common.hpp"
#include <bit>
#include <random>
#include <type_traits>

// Harpocrates - An Efficient Encryption Mechanism for Data-at-rest, related
// utility functions
namespace harpocrates_utils {

// Fisher-Yates Shuffling Algorithm, used for shuffling provided Look Up
// Table ( read `lut` ), while attempting to use non-deterministic randomness
// source ( if available )
//
// See algorithm 5 of Harpocrates specification
// https://eprint.iacr.org/2022/519.pdf
static inline void
shuffle(uint8_t* const lut)
{
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint8_t> dis;

  constexpr uint8_t n = 255;
  for (uint8_t i = 0; i < n; i++) {
    const uint8_t min = i;
    constexpr uint8_t max = n;
    const uint8_t rnd = dis(gen);

    const uint8_t j = min + rnd % (max - min + 1);

    uint8_t a = lut[i];
    uint8_t b = lut[j];

    a ^= b;
    b ^= a;
    a ^= b;

    lut[i] = a;
    lut[j] = b;
  }
}

// Generation of Look Up Table ( read `lut` ), as defined in section 2.5 of
// Harpocrates specification https://eprint.iacr.org/2022/519.pdf
static inline void
generate_lut(uint8_t* const lut)
{
  constexpr size_t n = 256;
  for (size_t i = 0; i < n; i++) {
    lut[i] = i;
  }

  shuffle(lut);
}

// Generation of inverse Look Up Table ( read `inv_lut` ), using involution
// function of `lut`, as defined in section 2.1 of Harpocrates specification
// https://eprint.iacr.org/2022/519.pdf
static inline void
generate_inv_lut(const uint8_t* const __restrict lut,
                 uint8_t* const __restrict inv_lut)
{
  constexpr size_t n = 256;
  for (size_t i = 0; i < n; i++) {
    inv_lut[lut[i]] = i;
  }
}

// Left to right convoluted substitution, as described in algorithm 2 of
// Harpocrates specification https://eprint.iacr.org/2022/519.pdf
//
// Also see figure 4 of above linked document to better understand workings of
// this procedure
static inline void
left_to_right_convoluted_substitution(uint16_t* const __restrict state,
                                      const uint8_t* const __restrict lut)
{
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    uint16_t tmp = state[i];

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(tmp >> 8);
    const uint8_t t1 = lut[t0];

    tmp = (static_cast<uint16_t>(t1) << 8) | (tmp & 0b11111111u);

    // step 2
    const uint8_t t2 = static_cast<uint8_t>(tmp >> 6);
    const uint8_t t3 = lut[t2];

    tmp = (tmp & 0b1100000000000000u) | (static_cast<uint16_t>(t3) << 6) |
          (tmp & 0b111111u);

    // step 3
    const uint8_t t4 = static_cast<uint8_t>(tmp >> 4);
    const uint8_t t5 = lut[t4];

    tmp = (tmp & 0b1111000000000000u) | (static_cast<uint16_t>(t5) << 4) |
          (tmp & 0b1111u);

    // step 4
    const uint8_t t6 = static_cast<uint8_t>(tmp >> 2);
    const uint8_t t7 = lut[t6];

    tmp = (tmp & 0b1111110000000000u) | (static_cast<uint16_t>(t7) << 2) |
          (tmp & 0b11u);

    // step 5
    const uint8_t t8 = static_cast<uint8_t>(tmp);
    const uint8_t t9 = lut[t8];

    tmp = (tmp & 0b1111111100000000u) | static_cast<uint16_t>(t9);

    state[i] = tmp;
  }
}

// Adds round constants into state matrix, to break the round's self-similarity
//
// See `Round constant addition` point in section 2.3 of Harpocrates
// specification https://eprint.iacr.org/2022/519.pdf
static inline void
add_rc(uint16_t* const state, const size_t r_idx)
{
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    state[i] ^= std::rotl(harpocrates_common::RC[i], r_idx << 1);
  }
}

// Column substitution for diffusing value of each row, taken from algorithm 3
// described in section 2.3 of Harpocrates specification
// https://eprint.iacr.org/2022/519.pdf
static inline void
column_substitution(uint16_t* const __restrict state,
                    const uint8_t* const __restrict lut)
{
  for (size_t i = 0; i < harpocrates_common::N_COLS; i++) {
    const uint8_t b0 = static_cast<uint8_t>((state[0] >> (15ul - i)) & 0b1u);
    const uint8_t b1 = static_cast<uint8_t>((state[1] >> (15ul - i)) & 0b1u);
    const uint8_t b2 = static_cast<uint8_t>((state[2] >> (15ul - i)) & 0b1u);
    const uint8_t b3 = static_cast<uint8_t>((state[3] >> (15ul - i)) & 0b1u);
    const uint8_t b4 = static_cast<uint8_t>((state[4] >> (15ul - i)) & 0b1u);
    const uint8_t b5 = static_cast<uint8_t>((state[5] >> (15ul - i)) & 0b1u);
    const uint8_t b6 = static_cast<uint8_t>((state[6] >> (15ul - i)) & 0b1u);
    const uint8_t b7 = static_cast<uint8_t>((state[7] >> (15ul - i)) & 0b1u);

    const uint8_t col0 = (b0 << 7) | (b1 << 6) | (b2 << 5) | (b3 << 4) |
                         (b4 << 3) | (b5 << 2) | (b6 << 1) | (b7 << 0);

    const uint8_t col1 = lut[col0];

    state[0] = (state[0] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 7) & 0b1u) << (15ul - i));
    state[1] = (state[1] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 6) & 0b1u) << (15ul - i));
    state[2] = (state[2] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 5) & 0b1u) << (15ul - i));
    state[3] = (state[3] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 4) & 0b1u) << (15ul - i));
    state[4] = (state[4] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 3) & 0b1u) << (15ul - i));
    state[5] = (state[5] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 2) & 0b1u) << (15ul - i));
    state[6] = (state[6] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 1) & 0b1u) << (15ul - i));
    state[7] = (state[7] & harpocrates_common::COL_MASKS[i]) |
               (static_cast<uint16_t>((col1 >> 0) & 0b1u) << (15ul - i));
  }
}

// Right to left convoluted substitution, as described in point (4) of
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
//
// Also see figure 7 of above linked document to better understand workings of
// this procedure
static inline void
right_to_left_convoluted_substitution(uint16_t* const __restrict state,
                                      const uint8_t* const __restrict lut)
{
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    uint16_t tmp = state[i];

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(tmp);
    const uint8_t t1 = lut[t0];

    tmp = (tmp & 0b1111111100000000u) | static_cast<uint16_t>(t1);

    // step 2
    const uint8_t t2 = static_cast<uint8_t>(tmp >> 2);
    const uint8_t t3 = lut[t2];

    tmp = (tmp & 0b1111110000000000u) | (static_cast<uint16_t>(t3) << 2) |
          (tmp & 0b11u);

    // step 3
    const uint8_t t4 = static_cast<uint8_t>(tmp >> 4);
    const uint8_t t5 = lut[t4];

    tmp = (tmp & 0b1111000000000000u) | (static_cast<uint16_t>(t5) << 4) |
          (tmp & 0b1111u);

    // step 4
    const uint8_t t6 = static_cast<uint8_t>(tmp >> 6);
    const uint8_t t7 = lut[t6];

    tmp = (tmp & 0b1100000000000000u) | (static_cast<uint16_t>(t7) << 6) |
          (tmp & 0b111111u);

    // step 5
    const uint8_t t8 = static_cast<uint8_t>(tmp >> 8);
    const uint8_t t9 = lut[t8];

    tmp = (static_cast<uint16_t>(t9) << 8) | (tmp & 0b11111111u);

    state[i] = tmp;
  }
}

}
