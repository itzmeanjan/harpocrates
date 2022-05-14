#pragma once
#include "harpocrates_common.hpp"
#include <bit>
#include <random>
#include <type_traits>

#if defined HARPOCRATES_PARALLEL
#include <CL/sycl.hpp>
#endif

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
  constexpr size_t n = 64;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < n; i++) {
    const size_t off = i << 2;

    lut[off] = off;
    lut[off ^ 1] = off ^ 1;
    lut[off ^ 2] = off ^ 2;
    lut[off ^ 3] = off ^ 3;
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
  constexpr size_t n = 64;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < n; i++) {
    const size_t off = i << 2;

    inv_lut[lut[off]] = off;
    inv_lut[lut[off ^ 1]] = off ^ 1;
    inv_lut[lut[off ^ 2]] = off ^ 2;
    inv_lut[lut[off ^ 3]] = off ^ 3;
  }
}

// Left to right convoluted substitution, as described in algorithm 2 of
// Harpocrates specification https://eprint.iacr.org/2022/519.pdf
//
// Also see figure 4 of above linked document to better understand workings of
// this procedure
#if defined HARPOCRATES_PARALLEL
static inline void
left_to_right_convoluted_substitution(uint16_t* const __restrict state,
                                      const sycl::local_ptr<uint8_t> lut)
{
#if defined __clang__
#pragma unroll 8
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 8
#endif
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    const uint16_t row = state[i];

    const uint8_t lo = static_cast<uint8_t>(row);

    const uint8_t lo_msb0 = lo >> 6;
    const uint8_t lo_msb2 = (lo >> 4) & 0b11;
    const uint8_t lo_msb4 = (lo >> 2) & 0b11;
    const uint8_t lo_msb6 = lo & 0b11;

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(row >> 8);
    const uint8_t t1 = lut[t0];
    const uint8_t msb0 = t1 & 0b11000000;

    // step 2
    const uint8_t t2 = (t1 << 2) | lo_msb0;
    const uint8_t t3 = lut[t2];
    const uint8_t msb2 = (t3 & 0b11000000) >> 2;

    // step 3
    const uint8_t t4 = (t3 << 2) | lo_msb2;
    const uint8_t t5 = lut[t4];
    const uint8_t msb4 = (t5 & 0b11000000) >> 4;

    // step 4
    const uint8_t t6 = (t5 << 2) | lo_msb4;
    const uint8_t t7 = lut[t6];
    const uint8_t msb6 = (t7 & 0b11000000) >> 6;

    // step 5
    const uint8_t t8 = (t7 << 2) | lo_msb6;
    const uint8_t t9 = lut[t8];

    const uint8_t hi = msb0 | msb2 | msb4 | msb6;
    state[i] = (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(t9);
  }
}
#else
static inline void
left_to_right_convoluted_substitution(uint16_t* const __restrict state,
                                      const uint8_t* const __restrict lut)
{
#if defined __clang__
#pragma unroll 8
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 8
#endif
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    const uint16_t row = state[i];

    const uint8_t lo = static_cast<uint8_t>(row);

    const uint8_t lo_msb0 = lo >> 6;
    const uint8_t lo_msb2 = (lo >> 4) & 0b11;
    const uint8_t lo_msb4 = (lo >> 2) & 0b11;
    const uint8_t lo_msb6 = lo & 0b11;

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(row >> 8);
    const uint8_t t1 = lut[t0];
    const uint8_t msb0 = t1 & 0b11000000;

    // step 2
    const uint8_t t2 = (t1 << 2) | lo_msb0;
    const uint8_t t3 = lut[t2];
    const uint8_t msb2 = (t3 & 0b11000000) >> 2;

    // step 3
    const uint8_t t4 = (t3 << 2) | lo_msb2;
    const uint8_t t5 = lut[t4];
    const uint8_t msb4 = (t5 & 0b11000000) >> 4;

    // step 4
    const uint8_t t6 = (t5 << 2) | lo_msb4;
    const uint8_t t7 = lut[t6];
    const uint8_t msb6 = (t7 & 0b11000000) >> 6;

    // step 5
    const uint8_t t8 = (t7 << 2) | lo_msb6;
    const uint8_t t9 = lut[t8];

    const uint8_t hi = msb0 | msb2 | msb4 | msb6;
    state[i] = (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(t9);
  }
}
#endif

// Adds round constants into state matrix, to break the round's self-similarity
//
// See `Round constant addition` point in section 2.3 of Harpocrates
// specification https://eprint.iacr.org/2022/519.pdf
static inline void
add_rc(uint16_t* const state, const size_t r_idx)
{
#if defined __clang__
#pragma unroll 8
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 8
#endif
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    state[i] ^= std::rotl(harpocrates_common::RC[i], r_idx << 1);
  }
}

// Column substitution for diffusing value of each row, taken from algorithm 3
// described in section 2.3 of Harpocrates specification
// https://eprint.iacr.org/2022/519.pdf
#if defined HARPOCRATES_PARALLEL
static inline void
column_substitution(uint16_t* const __restrict state,
                    const sycl::local_ptr<uint8_t> lut)
{
  const uint8_t row0_hi = static_cast<uint8_t>(state[0] >> 8);
  const uint8_t row1_hi = static_cast<uint8_t>(state[1] >> 8);
  const uint8_t row2_hi = static_cast<uint8_t>(state[2] >> 8);
  const uint8_t row3_hi = static_cast<uint8_t>(state[3] >> 8);
  const uint8_t row4_hi = static_cast<uint8_t>(state[4] >> 8);
  const uint8_t row5_hi = static_cast<uint8_t>(state[5] >> 8);
  const uint8_t row6_hi = static_cast<uint8_t>(state[6] >> 8);
  const uint8_t row7_hi = static_cast<uint8_t>(state[7] >> 8);

  const uint8_t col0 =
    ((row0_hi & 0b10000000) >> 0) | ((row1_hi & 0b10000000) >> 1) |
    ((row2_hi & 0b10000000) >> 2) | ((row3_hi & 0b10000000) >> 3) |
    ((row4_hi & 0b10000000) >> 4) | ((row5_hi & 0b10000000) >> 5) |
    ((row6_hi & 0b10000000) >> 6) | ((row7_hi & 0b10000000) >> 7);
  const uint8_t scol0 = lut[col0];

  const uint8_t col1 =
    ((row0_hi & 0b01000000) << 1) | ((row1_hi & 0b01000000) >> 0) |
    ((row2_hi & 0b01000000) >> 1) | ((row3_hi & 0b01000000) >> 2) |
    ((row4_hi & 0b01000000) >> 3) | ((row5_hi & 0b01000000) >> 4) |
    ((row6_hi & 0b01000000) >> 5) | ((row7_hi & 0b01000000) >> 6);
  const uint8_t scol1 = lut[col1];

  const uint8_t col2 =
    ((row0_hi & 0b00100000) << 2) | ((row1_hi & 0b00100000) << 1) |
    ((row2_hi & 0b00100000) >> 0) | ((row3_hi & 0b00100000) >> 1) |
    ((row4_hi & 0b00100000) >> 2) | ((row5_hi & 0b00100000) >> 3) |
    ((row6_hi & 0b00100000) >> 4) | ((row7_hi & 0b00100000) >> 5);
  const uint8_t scol2 = lut[col2];

  const uint8_t col3 =
    ((row0_hi & 0b00010000) << 3) | ((row1_hi & 0b00010000) << 2) |
    ((row2_hi & 0b00010000) << 1) | ((row3_hi & 0b00010000) >> 0) |
    ((row4_hi & 0b00010000) >> 1) | ((row5_hi & 0b00010000) >> 2) |
    ((row6_hi & 0b00010000) >> 3) | ((row7_hi & 0b00010000) >> 4);
  const uint8_t scol3 = lut[col3];

  const uint8_t col4 =
    ((row0_hi & 0b00001000) << 4) | ((row1_hi & 0b00001000) << 3) |
    ((row2_hi & 0b00001000) << 2) | ((row3_hi & 0b00001000) << 1) |
    ((row4_hi & 0b00001000) >> 0) | ((row5_hi & 0b00001000) >> 1) |
    ((row6_hi & 0b00001000) >> 2) | ((row7_hi & 0b00001000) >> 3);
  const uint8_t scol4 = lut[col4];

  const uint8_t col5 =
    ((row0_hi & 0b00000100) << 5) | ((row1_hi & 0b00000100) << 4) |
    ((row2_hi & 0b00000100) << 3) | ((row3_hi & 0b00000100) << 2) |
    ((row4_hi & 0b00000100) << 1) | ((row5_hi & 0b00000100) >> 0) |
    ((row6_hi & 0b00000100) >> 1) | ((row7_hi & 0b00000100) >> 2);
  const uint8_t scol5 = lut[col5];

  const uint8_t col6 =
    ((row0_hi & 0b00000010) << 6) | ((row1_hi & 0b00000010) << 5) |
    ((row2_hi & 0b00000010) << 4) | ((row3_hi & 0b00000010) << 3) |
    ((row4_hi & 0b00000010) << 2) | ((row5_hi & 0b00000010) << 1) |
    ((row6_hi & 0b00000010) >> 0) | ((row7_hi & 0b00000010) >> 1);
  const uint8_t scol6 = lut[col6];

  const uint8_t col7 =
    ((row0_hi & 0b00000001) << 7) | ((row1_hi & 0b00000001) << 6) |
    ((row2_hi & 0b00000001) << 5) | ((row3_hi & 0b00000001) << 4) |
    ((row4_hi & 0b00000001) << 3) | ((row5_hi & 0b00000001) << 2) |
    ((row6_hi & 0b00000001) << 1) | ((row7_hi & 0b00000001) << 0);
  const uint8_t scol7 = lut[col7];

  const uint8_t row0_lo = static_cast<uint8_t>(state[0]);
  const uint8_t row1_lo = static_cast<uint8_t>(state[1]);
  const uint8_t row2_lo = static_cast<uint8_t>(state[2]);
  const uint8_t row3_lo = static_cast<uint8_t>(state[3]);
  const uint8_t row4_lo = static_cast<uint8_t>(state[4]);
  const uint8_t row5_lo = static_cast<uint8_t>(state[5]);
  const uint8_t row6_lo = static_cast<uint8_t>(state[6]);
  const uint8_t row7_lo = static_cast<uint8_t>(state[7]);

  const uint8_t col8 =
    ((row0_lo & 0b10000000) >> 0) | ((row1_lo & 0b10000000) >> 1) |
    ((row2_lo & 0b10000000) >> 2) | ((row3_lo & 0b10000000) >> 3) |
    ((row4_lo & 0b10000000) >> 4) | ((row5_lo & 0b10000000) >> 5) |
    ((row6_lo & 0b10000000) >> 6) | ((row7_lo & 0b10000000) >> 7);
  const uint8_t scol8 = lut[col8];

  const uint8_t col9 =
    ((row0_lo & 0b01000000) << 1) | ((row1_lo & 0b01000000) >> 0) |
    ((row2_lo & 0b01000000) >> 1) | ((row3_lo & 0b01000000) >> 2) |
    ((row4_lo & 0b01000000) >> 3) | ((row5_lo & 0b01000000) >> 4) |
    ((row6_lo & 0b01000000) >> 5) | ((row7_lo & 0b01000000) >> 6);
  const uint8_t scol9 = lut[col9];

  const uint8_t col10 =
    ((row0_lo & 0b00100000) << 2) | ((row1_lo & 0b00100000) << 1) |
    ((row2_lo & 0b00100000) >> 0) | ((row3_lo & 0b00100000) >> 1) |
    ((row4_lo & 0b00100000) >> 2) | ((row5_lo & 0b00100000) >> 3) |
    ((row6_lo & 0b00100000) >> 4) | ((row7_lo & 0b00100000) >> 5);
  const uint8_t scol10 = lut[col10];

  const uint8_t col11 =
    ((row0_lo & 0b00010000) << 3) | ((row1_lo & 0b00010000) << 2) |
    ((row2_lo & 0b00010000) << 1) | ((row3_lo & 0b00010000) >> 0) |
    ((row4_lo & 0b00010000) >> 1) | ((row5_lo & 0b00010000) >> 2) |
    ((row6_lo & 0b00010000) >> 3) | ((row7_lo & 0b00010000) >> 4);
  const uint8_t scol11 = lut[col11];

  const uint8_t col12 =
    ((row0_lo & 0b00001000) << 4) | ((row1_lo & 0b00001000) << 3) |
    ((row2_lo & 0b00001000) << 2) | ((row3_lo & 0b00001000) << 1) |
    ((row4_lo & 0b00001000) >> 0) | ((row5_lo & 0b00001000) >> 1) |
    ((row6_lo & 0b00001000) >> 2) | ((row7_lo & 0b00001000) >> 3);
  const uint8_t scol12 = lut[col12];

  const uint8_t col13 =
    ((row0_lo & 0b00000100) << 5) | ((row1_lo & 0b00000100) << 4) |
    ((row2_lo & 0b00000100) << 3) | ((row3_lo & 0b00000100) << 2) |
    ((row4_lo & 0b00000100) << 1) | ((row5_lo & 0b00000100) >> 0) |
    ((row6_lo & 0b00000100) >> 1) | ((row7_lo & 0b00000100) >> 2);
  const uint8_t scol13 = lut[col13];

  const uint8_t col14 =
    ((row0_lo & 0b00000010) << 6) | ((row1_lo & 0b00000010) << 5) |
    ((row2_lo & 0b00000010) << 4) | ((row3_lo & 0b00000010) << 3) |
    ((row4_lo & 0b00000010) << 2) | ((row5_lo & 0b00000010) << 1) |
    ((row6_lo & 0b00000010) >> 0) | ((row7_lo & 0b00000010) >> 1);
  const uint8_t scol14 = lut[col14];

  const uint8_t col15 =
    ((row0_lo & 0b00000001) << 7) | ((row1_lo & 0b00000001) << 6) |
    ((row2_lo & 0b00000001) << 5) | ((row3_lo & 0b00000001) << 4) |
    ((row4_lo & 0b00000001) << 3) | ((row5_lo & 0b00000001) << 2) |
    ((row6_lo & 0b00000001) << 1) | ((row7_lo & 0b00000001) << 0);
  const uint8_t scol15 = lut[col15];

  const uint16_t row0 = (static_cast<uint16_t>(scol0 & 0b10000000) << 8) |
                        (static_cast<uint16_t>(scol1 & 0b10000000) << 7) |
                        (static_cast<uint16_t>(scol2 & 0b10000000) << 6) |
                        (static_cast<uint16_t>(scol3 & 0b10000000) << 5) |
                        (static_cast<uint16_t>(scol4 & 0b10000000) << 4) |
                        (static_cast<uint16_t>(scol5 & 0b10000000) << 3) |
                        (static_cast<uint16_t>(scol6 & 0b10000000) << 2) |
                        (static_cast<uint16_t>(scol7 & 0b10000000) << 1) |
                        (static_cast<uint16_t>(scol8 & 0b10000000) >> 0) |
                        (static_cast<uint16_t>(scol9 & 0b10000000) >> 1) |
                        (static_cast<uint16_t>(scol10 & 0b10000000) >> 2) |
                        (static_cast<uint16_t>(scol11 & 0b10000000) >> 3) |
                        (static_cast<uint16_t>(scol12 & 0b10000000) >> 4) |
                        (static_cast<uint16_t>(scol13 & 0b10000000) >> 5) |
                        (static_cast<uint16_t>(scol14 & 0b10000000) >> 6) |
                        (static_cast<uint16_t>(scol15 & 0b10000000) >> 7);

  const uint16_t row1 = (static_cast<uint16_t>(scol0 & 0b01000000) << 9) |
                        (static_cast<uint16_t>(scol1 & 0b01000000) << 8) |
                        (static_cast<uint16_t>(scol2 & 0b01000000) << 7) |
                        (static_cast<uint16_t>(scol3 & 0b01000000) << 6) |
                        (static_cast<uint16_t>(scol4 & 0b01000000) << 5) |
                        (static_cast<uint16_t>(scol5 & 0b01000000) << 4) |
                        (static_cast<uint16_t>(scol6 & 0b01000000) << 3) |
                        (static_cast<uint16_t>(scol7 & 0b01000000) << 2) |
                        (static_cast<uint16_t>(scol8 & 0b01000000) << 1) |
                        (static_cast<uint16_t>(scol9 & 0b01000000) >> 0) |
                        (static_cast<uint16_t>(scol10 & 0b01000000) >> 1) |
                        (static_cast<uint16_t>(scol11 & 0b01000000) >> 2) |
                        (static_cast<uint16_t>(scol12 & 0b01000000) >> 3) |
                        (static_cast<uint16_t>(scol13 & 0b01000000) >> 4) |
                        (static_cast<uint16_t>(scol14 & 0b01000000) >> 5) |
                        (static_cast<uint16_t>(scol15 & 0b01000000) >> 6);

  const uint16_t row2 = (static_cast<uint16_t>(scol0 & 0b00100000) << 10) |
                        (static_cast<uint16_t>(scol1 & 0b00100000) << 9) |
                        (static_cast<uint16_t>(scol2 & 0b00100000) << 8) |
                        (static_cast<uint16_t>(scol3 & 0b00100000) << 7) |
                        (static_cast<uint16_t>(scol4 & 0b00100000) << 6) |
                        (static_cast<uint16_t>(scol5 & 0b00100000) << 5) |
                        (static_cast<uint16_t>(scol6 & 0b00100000) << 4) |
                        (static_cast<uint16_t>(scol7 & 0b00100000) << 3) |
                        (static_cast<uint16_t>(scol8 & 0b00100000) << 2) |
                        (static_cast<uint16_t>(scol9 & 0b00100000) << 1) |
                        (static_cast<uint16_t>(scol10 & 0b00100000) >> 0) |
                        (static_cast<uint16_t>(scol11 & 0b00100000) >> 1) |
                        (static_cast<uint16_t>(scol12 & 0b00100000) >> 2) |
                        (static_cast<uint16_t>(scol13 & 0b00100000) >> 3) |
                        (static_cast<uint16_t>(scol14 & 0b00100000) >> 4) |
                        (static_cast<uint16_t>(scol15 & 0b00100000) >> 5);

  const uint16_t row3 = (static_cast<uint16_t>(scol0 & 0b00010000) << 11) |
                        (static_cast<uint16_t>(scol1 & 0b00010000) << 10) |
                        (static_cast<uint16_t>(scol2 & 0b00010000) << 9) |
                        (static_cast<uint16_t>(scol3 & 0b00010000) << 8) |
                        (static_cast<uint16_t>(scol4 & 0b00010000) << 7) |
                        (static_cast<uint16_t>(scol5 & 0b00010000) << 6) |
                        (static_cast<uint16_t>(scol6 & 0b00010000) << 5) |
                        (static_cast<uint16_t>(scol7 & 0b00010000) << 4) |
                        (static_cast<uint16_t>(scol8 & 0b00010000) << 3) |
                        (static_cast<uint16_t>(scol9 & 0b00010000) << 2) |
                        (static_cast<uint16_t>(scol10 & 0b00010000) << 1) |
                        (static_cast<uint16_t>(scol11 & 0b00010000) >> 0) |
                        (static_cast<uint16_t>(scol12 & 0b00010000) >> 1) |
                        (static_cast<uint16_t>(scol13 & 0b00010000) >> 2) |
                        (static_cast<uint16_t>(scol14 & 0b00010000) >> 3) |
                        (static_cast<uint16_t>(scol15 & 0b00010000) >> 4);

  const uint16_t row4 = (static_cast<uint16_t>(scol0 & 0b00001000) << 12) |
                        (static_cast<uint16_t>(scol1 & 0b00001000) << 11) |
                        (static_cast<uint16_t>(scol2 & 0b00001000) << 10) |
                        (static_cast<uint16_t>(scol3 & 0b00001000) << 9) |
                        (static_cast<uint16_t>(scol4 & 0b00001000) << 8) |
                        (static_cast<uint16_t>(scol5 & 0b00001000) << 7) |
                        (static_cast<uint16_t>(scol6 & 0b00001000) << 6) |
                        (static_cast<uint16_t>(scol7 & 0b00001000) << 5) |
                        (static_cast<uint16_t>(scol8 & 0b00001000) << 4) |
                        (static_cast<uint16_t>(scol9 & 0b00001000) << 3) |
                        (static_cast<uint16_t>(scol10 & 0b00001000) << 2) |
                        (static_cast<uint16_t>(scol11 & 0b00001000) << 1) |
                        (static_cast<uint16_t>(scol12 & 0b00001000) >> 0) |
                        (static_cast<uint16_t>(scol13 & 0b00001000) >> 1) |
                        (static_cast<uint16_t>(scol14 & 0b00001000) >> 2) |
                        (static_cast<uint16_t>(scol15 & 0b00001000) >> 3);

  const uint16_t row5 = (static_cast<uint16_t>(scol0 & 0b00000100) << 13) |
                        (static_cast<uint16_t>(scol1 & 0b00000100) << 12) |
                        (static_cast<uint16_t>(scol2 & 0b00000100) << 11) |
                        (static_cast<uint16_t>(scol3 & 0b00000100) << 10) |
                        (static_cast<uint16_t>(scol4 & 0b00000100) << 9) |
                        (static_cast<uint16_t>(scol5 & 0b00000100) << 8) |
                        (static_cast<uint16_t>(scol6 & 0b00000100) << 7) |
                        (static_cast<uint16_t>(scol7 & 0b00000100) << 6) |
                        (static_cast<uint16_t>(scol8 & 0b00000100) << 5) |
                        (static_cast<uint16_t>(scol9 & 0b00000100) << 4) |
                        (static_cast<uint16_t>(scol10 & 0b00000100) << 3) |
                        (static_cast<uint16_t>(scol11 & 0b00000100) << 2) |
                        (static_cast<uint16_t>(scol12 & 0b00000100) << 1) |
                        (static_cast<uint16_t>(scol13 & 0b00000100) >> 0) |
                        (static_cast<uint16_t>(scol14 & 0b00000100) >> 1) |
                        (static_cast<uint16_t>(scol15 & 0b00000100) >> 2);

  const uint16_t row6 = (static_cast<uint16_t>(scol0 & 0b00000010) << 14) |
                        (static_cast<uint16_t>(scol1 & 0b00000010) << 13) |
                        (static_cast<uint16_t>(scol2 & 0b00000010) << 12) |
                        (static_cast<uint16_t>(scol3 & 0b00000010) << 11) |
                        (static_cast<uint16_t>(scol4 & 0b00000010) << 10) |
                        (static_cast<uint16_t>(scol5 & 0b00000010) << 9) |
                        (static_cast<uint16_t>(scol6 & 0b00000010) << 8) |
                        (static_cast<uint16_t>(scol7 & 0b00000010) << 7) |
                        (static_cast<uint16_t>(scol8 & 0b00000010) << 6) |
                        (static_cast<uint16_t>(scol9 & 0b00000010) << 5) |
                        (static_cast<uint16_t>(scol10 & 0b00000010) << 4) |
                        (static_cast<uint16_t>(scol11 & 0b00000010) << 3) |
                        (static_cast<uint16_t>(scol12 & 0b00000010) << 2) |
                        (static_cast<uint16_t>(scol13 & 0b00000010) << 1) |
                        (static_cast<uint16_t>(scol14 & 0b00000010) >> 0) |
                        (static_cast<uint16_t>(scol15 & 0b00000010) >> 1);

  const uint16_t row7 = (static_cast<uint16_t>(scol0 & 0b00000001) << 15) |
                        (static_cast<uint16_t>(scol1 & 0b00000001) << 14) |
                        (static_cast<uint16_t>(scol2 & 0b00000001) << 13) |
                        (static_cast<uint16_t>(scol3 & 0b00000001) << 12) |
                        (static_cast<uint16_t>(scol4 & 0b00000001) << 11) |
                        (static_cast<uint16_t>(scol5 & 0b00000001) << 10) |
                        (static_cast<uint16_t>(scol6 & 0b00000001) << 9) |
                        (static_cast<uint16_t>(scol7 & 0b00000001) << 8) |
                        (static_cast<uint16_t>(scol8 & 0b00000001) << 7) |
                        (static_cast<uint16_t>(scol9 & 0b00000001) << 6) |
                        (static_cast<uint16_t>(scol10 & 0b00000001) << 5) |
                        (static_cast<uint16_t>(scol11 & 0b00000001) << 4) |
                        (static_cast<uint16_t>(scol12 & 0b00000001) << 3) |
                        (static_cast<uint16_t>(scol13 & 0b00000001) << 2) |
                        (static_cast<uint16_t>(scol14 & 0b00000001) << 1) |
                        (static_cast<uint16_t>(scol15 & 0b00000001) >> 0);

  state[0] = row0;
  state[1] = row1;
  state[2] = row2;
  state[3] = row3;
  state[4] = row4;
  state[5] = row5;
  state[6] = row6;
  state[7] = row7;
}
#else
static inline void
column_substitution(uint16_t* const __restrict state,
                    const uint8_t* const __restrict lut)
{
  const uint8_t row0_hi = static_cast<uint8_t>(state[0] >> 8);
  const uint8_t row1_hi = static_cast<uint8_t>(state[1] >> 8);
  const uint8_t row2_hi = static_cast<uint8_t>(state[2] >> 8);
  const uint8_t row3_hi = static_cast<uint8_t>(state[3] >> 8);
  const uint8_t row4_hi = static_cast<uint8_t>(state[4] >> 8);
  const uint8_t row5_hi = static_cast<uint8_t>(state[5] >> 8);
  const uint8_t row6_hi = static_cast<uint8_t>(state[6] >> 8);
  const uint8_t row7_hi = static_cast<uint8_t>(state[7] >> 8);

  const uint8_t col0 =
    ((row0_hi & 0b10000000) >> 0) | ((row1_hi & 0b10000000) >> 1) |
    ((row2_hi & 0b10000000) >> 2) | ((row3_hi & 0b10000000) >> 3) |
    ((row4_hi & 0b10000000) >> 4) | ((row5_hi & 0b10000000) >> 5) |
    ((row6_hi & 0b10000000) >> 6) | ((row7_hi & 0b10000000) >> 7);
  const uint8_t scol0 = lut[col0];

  const uint8_t col1 =
    ((row0_hi & 0b01000000) << 1) | ((row1_hi & 0b01000000) >> 0) |
    ((row2_hi & 0b01000000) >> 1) | ((row3_hi & 0b01000000) >> 2) |
    ((row4_hi & 0b01000000) >> 3) | ((row5_hi & 0b01000000) >> 4) |
    ((row6_hi & 0b01000000) >> 5) | ((row7_hi & 0b01000000) >> 6);
  const uint8_t scol1 = lut[col1];

  const uint8_t col2 =
    ((row0_hi & 0b00100000) << 2) | ((row1_hi & 0b00100000) << 1) |
    ((row2_hi & 0b00100000) >> 0) | ((row3_hi & 0b00100000) >> 1) |
    ((row4_hi & 0b00100000) >> 2) | ((row5_hi & 0b00100000) >> 3) |
    ((row6_hi & 0b00100000) >> 4) | ((row7_hi & 0b00100000) >> 5);
  const uint8_t scol2 = lut[col2];

  const uint8_t col3 =
    ((row0_hi & 0b00010000) << 3) | ((row1_hi & 0b00010000) << 2) |
    ((row2_hi & 0b00010000) << 1) | ((row3_hi & 0b00010000) >> 0) |
    ((row4_hi & 0b00010000) >> 1) | ((row5_hi & 0b00010000) >> 2) |
    ((row6_hi & 0b00010000) >> 3) | ((row7_hi & 0b00010000) >> 4);
  const uint8_t scol3 = lut[col3];

  const uint8_t col4 =
    ((row0_hi & 0b00001000) << 4) | ((row1_hi & 0b00001000) << 3) |
    ((row2_hi & 0b00001000) << 2) | ((row3_hi & 0b00001000) << 1) |
    ((row4_hi & 0b00001000) >> 0) | ((row5_hi & 0b00001000) >> 1) |
    ((row6_hi & 0b00001000) >> 2) | ((row7_hi & 0b00001000) >> 3);
  const uint8_t scol4 = lut[col4];

  const uint8_t col5 =
    ((row0_hi & 0b00000100) << 5) | ((row1_hi & 0b00000100) << 4) |
    ((row2_hi & 0b00000100) << 3) | ((row3_hi & 0b00000100) << 2) |
    ((row4_hi & 0b00000100) << 1) | ((row5_hi & 0b00000100) >> 0) |
    ((row6_hi & 0b00000100) >> 1) | ((row7_hi & 0b00000100) >> 2);
  const uint8_t scol5 = lut[col5];

  const uint8_t col6 =
    ((row0_hi & 0b00000010) << 6) | ((row1_hi & 0b00000010) << 5) |
    ((row2_hi & 0b00000010) << 4) | ((row3_hi & 0b00000010) << 3) |
    ((row4_hi & 0b00000010) << 2) | ((row5_hi & 0b00000010) << 1) |
    ((row6_hi & 0b00000010) >> 0) | ((row7_hi & 0b00000010) >> 1);
  const uint8_t scol6 = lut[col6];

  const uint8_t col7 =
    ((row0_hi & 0b00000001) << 7) | ((row1_hi & 0b00000001) << 6) |
    ((row2_hi & 0b00000001) << 5) | ((row3_hi & 0b00000001) << 4) |
    ((row4_hi & 0b00000001) << 3) | ((row5_hi & 0b00000001) << 2) |
    ((row6_hi & 0b00000001) << 1) | ((row7_hi & 0b00000001) << 0);
  const uint8_t scol7 = lut[col7];

  const uint8_t row0_lo = static_cast<uint8_t>(state[0]);
  const uint8_t row1_lo = static_cast<uint8_t>(state[1]);
  const uint8_t row2_lo = static_cast<uint8_t>(state[2]);
  const uint8_t row3_lo = static_cast<uint8_t>(state[3]);
  const uint8_t row4_lo = static_cast<uint8_t>(state[4]);
  const uint8_t row5_lo = static_cast<uint8_t>(state[5]);
  const uint8_t row6_lo = static_cast<uint8_t>(state[6]);
  const uint8_t row7_lo = static_cast<uint8_t>(state[7]);

  const uint8_t col8 =
    ((row0_lo & 0b10000000) >> 0) | ((row1_lo & 0b10000000) >> 1) |
    ((row2_lo & 0b10000000) >> 2) | ((row3_lo & 0b10000000) >> 3) |
    ((row4_lo & 0b10000000) >> 4) | ((row5_lo & 0b10000000) >> 5) |
    ((row6_lo & 0b10000000) >> 6) | ((row7_lo & 0b10000000) >> 7);
  const uint8_t scol8 = lut[col8];

  const uint8_t col9 =
    ((row0_lo & 0b01000000) << 1) | ((row1_lo & 0b01000000) >> 0) |
    ((row2_lo & 0b01000000) >> 1) | ((row3_lo & 0b01000000) >> 2) |
    ((row4_lo & 0b01000000) >> 3) | ((row5_lo & 0b01000000) >> 4) |
    ((row6_lo & 0b01000000) >> 5) | ((row7_lo & 0b01000000) >> 6);
  const uint8_t scol9 = lut[col9];

  const uint8_t col10 =
    ((row0_lo & 0b00100000) << 2) | ((row1_lo & 0b00100000) << 1) |
    ((row2_lo & 0b00100000) >> 0) | ((row3_lo & 0b00100000) >> 1) |
    ((row4_lo & 0b00100000) >> 2) | ((row5_lo & 0b00100000) >> 3) |
    ((row6_lo & 0b00100000) >> 4) | ((row7_lo & 0b00100000) >> 5);
  const uint8_t scol10 = lut[col10];

  const uint8_t col11 =
    ((row0_lo & 0b00010000) << 3) | ((row1_lo & 0b00010000) << 2) |
    ((row2_lo & 0b00010000) << 1) | ((row3_lo & 0b00010000) >> 0) |
    ((row4_lo & 0b00010000) >> 1) | ((row5_lo & 0b00010000) >> 2) |
    ((row6_lo & 0b00010000) >> 3) | ((row7_lo & 0b00010000) >> 4);
  const uint8_t scol11 = lut[col11];

  const uint8_t col12 =
    ((row0_lo & 0b00001000) << 4) | ((row1_lo & 0b00001000) << 3) |
    ((row2_lo & 0b00001000) << 2) | ((row3_lo & 0b00001000) << 1) |
    ((row4_lo & 0b00001000) >> 0) | ((row5_lo & 0b00001000) >> 1) |
    ((row6_lo & 0b00001000) >> 2) | ((row7_lo & 0b00001000) >> 3);
  const uint8_t scol12 = lut[col12];

  const uint8_t col13 =
    ((row0_lo & 0b00000100) << 5) | ((row1_lo & 0b00000100) << 4) |
    ((row2_lo & 0b00000100) << 3) | ((row3_lo & 0b00000100) << 2) |
    ((row4_lo & 0b00000100) << 1) | ((row5_lo & 0b00000100) >> 0) |
    ((row6_lo & 0b00000100) >> 1) | ((row7_lo & 0b00000100) >> 2);
  const uint8_t scol13 = lut[col13];

  const uint8_t col14 =
    ((row0_lo & 0b00000010) << 6) | ((row1_lo & 0b00000010) << 5) |
    ((row2_lo & 0b00000010) << 4) | ((row3_lo & 0b00000010) << 3) |
    ((row4_lo & 0b00000010) << 2) | ((row5_lo & 0b00000010) << 1) |
    ((row6_lo & 0b00000010) >> 0) | ((row7_lo & 0b00000010) >> 1);
  const uint8_t scol14 = lut[col14];

  const uint8_t col15 =
    ((row0_lo & 0b00000001) << 7) | ((row1_lo & 0b00000001) << 6) |
    ((row2_lo & 0b00000001) << 5) | ((row3_lo & 0b00000001) << 4) |
    ((row4_lo & 0b00000001) << 3) | ((row5_lo & 0b00000001) << 2) |
    ((row6_lo & 0b00000001) << 1) | ((row7_lo & 0b00000001) << 0);
  const uint8_t scol15 = lut[col15];

  const uint16_t row0 = (static_cast<uint16_t>(scol0 & 0b10000000) << 8) |
                        (static_cast<uint16_t>(scol1 & 0b10000000) << 7) |
                        (static_cast<uint16_t>(scol2 & 0b10000000) << 6) |
                        (static_cast<uint16_t>(scol3 & 0b10000000) << 5) |
                        (static_cast<uint16_t>(scol4 & 0b10000000) << 4) |
                        (static_cast<uint16_t>(scol5 & 0b10000000) << 3) |
                        (static_cast<uint16_t>(scol6 & 0b10000000) << 2) |
                        (static_cast<uint16_t>(scol7 & 0b10000000) << 1) |
                        (static_cast<uint16_t>(scol8 & 0b10000000) >> 0) |
                        (static_cast<uint16_t>(scol9 & 0b10000000) >> 1) |
                        (static_cast<uint16_t>(scol10 & 0b10000000) >> 2) |
                        (static_cast<uint16_t>(scol11 & 0b10000000) >> 3) |
                        (static_cast<uint16_t>(scol12 & 0b10000000) >> 4) |
                        (static_cast<uint16_t>(scol13 & 0b10000000) >> 5) |
                        (static_cast<uint16_t>(scol14 & 0b10000000) >> 6) |
                        (static_cast<uint16_t>(scol15 & 0b10000000) >> 7);

  const uint16_t row1 = (static_cast<uint16_t>(scol0 & 0b01000000) << 9) |
                        (static_cast<uint16_t>(scol1 & 0b01000000) << 8) |
                        (static_cast<uint16_t>(scol2 & 0b01000000) << 7) |
                        (static_cast<uint16_t>(scol3 & 0b01000000) << 6) |
                        (static_cast<uint16_t>(scol4 & 0b01000000) << 5) |
                        (static_cast<uint16_t>(scol5 & 0b01000000) << 4) |
                        (static_cast<uint16_t>(scol6 & 0b01000000) << 3) |
                        (static_cast<uint16_t>(scol7 & 0b01000000) << 2) |
                        (static_cast<uint16_t>(scol8 & 0b01000000) << 1) |
                        (static_cast<uint16_t>(scol9 & 0b01000000) >> 0) |
                        (static_cast<uint16_t>(scol10 & 0b01000000) >> 1) |
                        (static_cast<uint16_t>(scol11 & 0b01000000) >> 2) |
                        (static_cast<uint16_t>(scol12 & 0b01000000) >> 3) |
                        (static_cast<uint16_t>(scol13 & 0b01000000) >> 4) |
                        (static_cast<uint16_t>(scol14 & 0b01000000) >> 5) |
                        (static_cast<uint16_t>(scol15 & 0b01000000) >> 6);

  const uint16_t row2 = (static_cast<uint16_t>(scol0 & 0b00100000) << 10) |
                        (static_cast<uint16_t>(scol1 & 0b00100000) << 9) |
                        (static_cast<uint16_t>(scol2 & 0b00100000) << 8) |
                        (static_cast<uint16_t>(scol3 & 0b00100000) << 7) |
                        (static_cast<uint16_t>(scol4 & 0b00100000) << 6) |
                        (static_cast<uint16_t>(scol5 & 0b00100000) << 5) |
                        (static_cast<uint16_t>(scol6 & 0b00100000) << 4) |
                        (static_cast<uint16_t>(scol7 & 0b00100000) << 3) |
                        (static_cast<uint16_t>(scol8 & 0b00100000) << 2) |
                        (static_cast<uint16_t>(scol9 & 0b00100000) << 1) |
                        (static_cast<uint16_t>(scol10 & 0b00100000) >> 0) |
                        (static_cast<uint16_t>(scol11 & 0b00100000) >> 1) |
                        (static_cast<uint16_t>(scol12 & 0b00100000) >> 2) |
                        (static_cast<uint16_t>(scol13 & 0b00100000) >> 3) |
                        (static_cast<uint16_t>(scol14 & 0b00100000) >> 4) |
                        (static_cast<uint16_t>(scol15 & 0b00100000) >> 5);

  const uint16_t row3 = (static_cast<uint16_t>(scol0 & 0b00010000) << 11) |
                        (static_cast<uint16_t>(scol1 & 0b00010000) << 10) |
                        (static_cast<uint16_t>(scol2 & 0b00010000) << 9) |
                        (static_cast<uint16_t>(scol3 & 0b00010000) << 8) |
                        (static_cast<uint16_t>(scol4 & 0b00010000) << 7) |
                        (static_cast<uint16_t>(scol5 & 0b00010000) << 6) |
                        (static_cast<uint16_t>(scol6 & 0b00010000) << 5) |
                        (static_cast<uint16_t>(scol7 & 0b00010000) << 4) |
                        (static_cast<uint16_t>(scol8 & 0b00010000) << 3) |
                        (static_cast<uint16_t>(scol9 & 0b00010000) << 2) |
                        (static_cast<uint16_t>(scol10 & 0b00010000) << 1) |
                        (static_cast<uint16_t>(scol11 & 0b00010000) >> 0) |
                        (static_cast<uint16_t>(scol12 & 0b00010000) >> 1) |
                        (static_cast<uint16_t>(scol13 & 0b00010000) >> 2) |
                        (static_cast<uint16_t>(scol14 & 0b00010000) >> 3) |
                        (static_cast<uint16_t>(scol15 & 0b00010000) >> 4);

  const uint16_t row4 = (static_cast<uint16_t>(scol0 & 0b00001000) << 12) |
                        (static_cast<uint16_t>(scol1 & 0b00001000) << 11) |
                        (static_cast<uint16_t>(scol2 & 0b00001000) << 10) |
                        (static_cast<uint16_t>(scol3 & 0b00001000) << 9) |
                        (static_cast<uint16_t>(scol4 & 0b00001000) << 8) |
                        (static_cast<uint16_t>(scol5 & 0b00001000) << 7) |
                        (static_cast<uint16_t>(scol6 & 0b00001000) << 6) |
                        (static_cast<uint16_t>(scol7 & 0b00001000) << 5) |
                        (static_cast<uint16_t>(scol8 & 0b00001000) << 4) |
                        (static_cast<uint16_t>(scol9 & 0b00001000) << 3) |
                        (static_cast<uint16_t>(scol10 & 0b00001000) << 2) |
                        (static_cast<uint16_t>(scol11 & 0b00001000) << 1) |
                        (static_cast<uint16_t>(scol12 & 0b00001000) >> 0) |
                        (static_cast<uint16_t>(scol13 & 0b00001000) >> 1) |
                        (static_cast<uint16_t>(scol14 & 0b00001000) >> 2) |
                        (static_cast<uint16_t>(scol15 & 0b00001000) >> 3);

  const uint16_t row5 = (static_cast<uint16_t>(scol0 & 0b00000100) << 13) |
                        (static_cast<uint16_t>(scol1 & 0b00000100) << 12) |
                        (static_cast<uint16_t>(scol2 & 0b00000100) << 11) |
                        (static_cast<uint16_t>(scol3 & 0b00000100) << 10) |
                        (static_cast<uint16_t>(scol4 & 0b00000100) << 9) |
                        (static_cast<uint16_t>(scol5 & 0b00000100) << 8) |
                        (static_cast<uint16_t>(scol6 & 0b00000100) << 7) |
                        (static_cast<uint16_t>(scol7 & 0b00000100) << 6) |
                        (static_cast<uint16_t>(scol8 & 0b00000100) << 5) |
                        (static_cast<uint16_t>(scol9 & 0b00000100) << 4) |
                        (static_cast<uint16_t>(scol10 & 0b00000100) << 3) |
                        (static_cast<uint16_t>(scol11 & 0b00000100) << 2) |
                        (static_cast<uint16_t>(scol12 & 0b00000100) << 1) |
                        (static_cast<uint16_t>(scol13 & 0b00000100) >> 0) |
                        (static_cast<uint16_t>(scol14 & 0b00000100) >> 1) |
                        (static_cast<uint16_t>(scol15 & 0b00000100) >> 2);

  const uint16_t row6 = (static_cast<uint16_t>(scol0 & 0b00000010) << 14) |
                        (static_cast<uint16_t>(scol1 & 0b00000010) << 13) |
                        (static_cast<uint16_t>(scol2 & 0b00000010) << 12) |
                        (static_cast<uint16_t>(scol3 & 0b00000010) << 11) |
                        (static_cast<uint16_t>(scol4 & 0b00000010) << 10) |
                        (static_cast<uint16_t>(scol5 & 0b00000010) << 9) |
                        (static_cast<uint16_t>(scol6 & 0b00000010) << 8) |
                        (static_cast<uint16_t>(scol7 & 0b00000010) << 7) |
                        (static_cast<uint16_t>(scol8 & 0b00000010) << 6) |
                        (static_cast<uint16_t>(scol9 & 0b00000010) << 5) |
                        (static_cast<uint16_t>(scol10 & 0b00000010) << 4) |
                        (static_cast<uint16_t>(scol11 & 0b00000010) << 3) |
                        (static_cast<uint16_t>(scol12 & 0b00000010) << 2) |
                        (static_cast<uint16_t>(scol13 & 0b00000010) << 1) |
                        (static_cast<uint16_t>(scol14 & 0b00000010) >> 0) |
                        (static_cast<uint16_t>(scol15 & 0b00000010) >> 1);

  const uint16_t row7 = (static_cast<uint16_t>(scol0 & 0b00000001) << 15) |
                        (static_cast<uint16_t>(scol1 & 0b00000001) << 14) |
                        (static_cast<uint16_t>(scol2 & 0b00000001) << 13) |
                        (static_cast<uint16_t>(scol3 & 0b00000001) << 12) |
                        (static_cast<uint16_t>(scol4 & 0b00000001) << 11) |
                        (static_cast<uint16_t>(scol5 & 0b00000001) << 10) |
                        (static_cast<uint16_t>(scol6 & 0b00000001) << 9) |
                        (static_cast<uint16_t>(scol7 & 0b00000001) << 8) |
                        (static_cast<uint16_t>(scol8 & 0b00000001) << 7) |
                        (static_cast<uint16_t>(scol9 & 0b00000001) << 6) |
                        (static_cast<uint16_t>(scol10 & 0b00000001) << 5) |
                        (static_cast<uint16_t>(scol11 & 0b00000001) << 4) |
                        (static_cast<uint16_t>(scol12 & 0b00000001) << 3) |
                        (static_cast<uint16_t>(scol13 & 0b00000001) << 2) |
                        (static_cast<uint16_t>(scol14 & 0b00000001) << 1) |
                        (static_cast<uint16_t>(scol15 & 0b00000001) >> 0);

  state[0] = row0;
  state[1] = row1;
  state[2] = row2;
  state[3] = row3;
  state[4] = row4;
  state[5] = row5;
  state[6] = row6;
  state[7] = row7;
}
#endif

// Right to left convoluted substitution, as described in point (4) of
// section 2.3 of Harpocrates specification https://eprint.iacr.org/2022/519.pdf
//
// Also see figure 7 of above linked document to better understand workings of
// this procedure
#if defined HARPOCRATES_PARALLEL
static inline void
right_to_left_convoluted_substitution(uint16_t* const __restrict state,
                                      const sycl::local_ptr<uint8_t> lut)
{
#if defined __clang__
#pragma unroll 8
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 8
#endif
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    const uint16_t row = state[i];

    const uint8_t hi = static_cast<uint8_t>(row >> 8);

    const uint8_t hi_msb6 = hi << 6;
    const uint8_t hi_msb4 = (hi << 4) & 0b11000000;
    const uint8_t hi_msb2 = (hi << 2) & 0b11000000;
    const uint8_t hi_msb0 = hi & 0b11000000;

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(row);
    const uint8_t t1 = lut[t0];
    const uint8_t msb6 = t1 & 0b11;

    // step 2
    const uint8_t t2 = hi_msb6 | (t1 >> 2);
    const uint8_t t3 = lut[t2];
    const uint8_t msb4 = (t3 & 0b11) << 2;

    // step 3
    const uint8_t t4 = hi_msb4 | (t3 >> 2);
    const uint8_t t5 = lut[t4];
    const uint8_t msb2 = (t5 & 0b11) << 4;

    // step 4
    const uint8_t t6 = hi_msb2 | (t5 >> 2);
    const uint8_t t7 = lut[t6];
    const uint8_t msb0 = (t7 & 0b11) << 6;

    // step 5
    const uint8_t t8 = hi_msb0 | (t7 >> 2);
    const uint8_t t9 = lut[t8];

    const uint8_t lo = msb0 | msb2 | msb4 | msb6;
    state[i] = (static_cast<uint16_t>(t9) << 8) | static_cast<uint16_t>(lo);
  }
}
#else
static inline void
right_to_left_convoluted_substitution(uint16_t* const __restrict state,
                                      const uint8_t* const __restrict lut)
{
#if defined __clang__
#pragma unroll 8
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 8
#endif
  for (size_t i = 0; i < harpocrates_common::N_ROWS; i++) {
    const uint16_t row = state[i];

    const uint8_t hi = static_cast<uint8_t>(row >> 8);

    const uint8_t hi_msb6 = hi << 6;
    const uint8_t hi_msb4 = (hi << 4) & 0b11000000;
    const uint8_t hi_msb2 = (hi << 2) & 0b11000000;
    const uint8_t hi_msb0 = hi & 0b11000000;

    // step 1
    const uint8_t t0 = static_cast<uint8_t>(row);
    const uint8_t t1 = lut[t0];
    const uint8_t msb6 = t1 & 0b11;

    // step 2
    const uint8_t t2 = hi_msb6 | (t1 >> 2);
    const uint8_t t3 = lut[t2];
    const uint8_t msb4 = (t3 & 0b11) << 2;

    // step 3
    const uint8_t t4 = hi_msb4 | (t3 >> 2);
    const uint8_t t5 = lut[t4];
    const uint8_t msb2 = (t5 & 0b11) << 4;

    // step 4
    const uint8_t t6 = hi_msb2 | (t5 >> 2);
    const uint8_t t7 = lut[t6];
    const uint8_t msb0 = (t7 & 0b11) << 6;

    // step 5
    const uint8_t t8 = hi_msb0 | (t7 >> 2);
    const uint8_t t9 = lut[t8];

    const uint8_t lo = msb0 | msb2 | msb4 | msb6;
    state[i] = (static_cast<uint16_t>(t9) << 8) | static_cast<uint16_t>(lo);
  }
}
#endif

}
