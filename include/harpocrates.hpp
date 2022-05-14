#pragma once
#include "harpocrates_utils.hpp"

// Harpocrates - An Efficient Encryption Mechanism for Data-at-rest
namespace harpocrates {

// Given 16 -bytes of unencrypted input message block & look up table
// ( read `lut` ) of size 256 ( because each LUT element is of size 8 -bit ),
// this routine computes 16 -bytes of encrypted data using Harpocrates
// Encryption algorithm
//
// Input:
// - lut: Look up table holding 256 elements
// - txt: 16 input bytes, to be encrypted
//
// Output:
// - enc: 16 encrypted output bytes
#if defined HARPOCRATES_PARALLEL
static inline void
encrypt(const sycl::local_ptr<uint8_t> lut,  // look up table
        const uint8_t* const __restrict txt, // input plain text
        uint8_t* const __restrict enc        // output encrypted bytes
)
{
  uint16_t state[8] = { 0u };

  constexpr size_t itr_cnt = harpocrates_common::N_ROWS >> 1;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    state[r_off ^ 0] = (static_cast<uint16_t>(txt[b_off ^ 0]) << 8) |
                       static_cast<uint16_t>(txt[b_off ^ 1]);
    state[r_off ^ 1] = (static_cast<uint16_t>(txt[b_off ^ 2]) << 8) |
                       static_cast<uint16_t>(txt[b_off ^ 3]);
  }

  for (size_t i = 0; i < harpocrates_common::N_ROUNDS; i++) {
    harpocrates_utils::left_to_right_convoluted_substitution(state, lut);
    harpocrates_utils::add_rc(state, i);
    harpocrates_utils::column_substitution(state, lut);
    harpocrates_utils::right_to_left_convoluted_substitution(state, lut);
  }

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    enc[b_off ^ 0] = static_cast<uint8_t>(state[r_off ^ 0] >> 8);
    enc[b_off ^ 1] = static_cast<uint8_t>(state[r_off ^ 0]);
    enc[b_off ^ 2] = static_cast<uint8_t>(state[r_off ^ 1] >> 8);
    enc[b_off ^ 3] = static_cast<uint8_t>(state[r_off ^ 1]);
  }
}
#else
static inline void
encrypt(const uint8_t* const __restrict lut, // look up table
        const uint8_t* const __restrict txt, // input plain text
        uint8_t* const __restrict enc        // output encrypted bytes
)
{
  uint16_t state[8] = { 0u };

  constexpr size_t itr_cnt = harpocrates_common::N_ROWS >> 1;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    state[r_off ^ 0] = (static_cast<uint16_t>(txt[b_off ^ 0]) << 8) |
                       static_cast<uint16_t>(txt[b_off ^ 1]);
    state[r_off ^ 1] = (static_cast<uint16_t>(txt[b_off ^ 2]) << 8) |
                       static_cast<uint16_t>(txt[b_off ^ 3]);
  }

  for (size_t i = 0; i < harpocrates_common::N_ROUNDS; i++) {
    harpocrates_utils::left_to_right_convoluted_substitution(state, lut);
    harpocrates_utils::add_rc(state, i);
    harpocrates_utils::column_substitution(state, lut);
    harpocrates_utils::right_to_left_convoluted_substitution(state, lut);
  }

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    enc[b_off ^ 0] = static_cast<uint8_t>(state[r_off ^ 0] >> 8);
    enc[b_off ^ 1] = static_cast<uint8_t>(state[r_off ^ 0]);
    enc[b_off ^ 2] = static_cast<uint8_t>(state[r_off ^ 1] >> 8);
    enc[b_off ^ 3] = static_cast<uint8_t>(state[r_off ^ 1]);
  }
}
#endif

// Given 16 -bytes of encrypted input message block & inverse look up table
// ( read `inv_lut` ) of size 256 ( because each LUT element is of size 8 -bit
// ), this routine computes 16 -bytes of unencrypted data using Harpocrates
// Decryption algorithm
//
// Input:
// - inv_lut: Inverse Look up table holding 256 elements
// - enc: 16 encrypted input bytes
//
// Output:
// - dec: 16 decrypted output bytes
//
// Note, inverse look up table needs to be computed as
//
// inv_lut = harpocrates_utils::generate_inv_lut(lut)
//
// where `lut` is the same look up table used during encryption.
#if defined HARPOCRATES_PARALLEL
static inline void
decrypt(const sycl::local_ptr<uint8_t> inv_lut, // inverse look up table
        const uint8_t* const __restrict enc,    // input encrypted bytes
        uint8_t* const __restrict dec           // output decrypted bytes
)
{
  uint16_t state[8] = { 0u };

  constexpr size_t itr_cnt = harpocrates_common::N_ROWS >> 1;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    state[r_off ^ 0] = (static_cast<uint16_t>(enc[b_off ^ 0]) << 8) |
                       static_cast<uint16_t>(enc[b_off ^ 1]);
    state[r_off ^ 1] = (static_cast<uint16_t>(enc[b_off ^ 2]) << 8) |
                       static_cast<uint16_t>(enc[b_off ^ 3]);
  }

  for (size_t i = 0; i < harpocrates_common::N_ROUNDS; i++) {
    using namespace harpocrates_utils;

    left_to_right_convoluted_substitution(state, inv_lut);
    column_substitution(state, inv_lut);
    add_rc(state, harpocrates_common::N_ROUNDS - (i + 1));
    right_to_left_convoluted_substitution(state, inv_lut);
  }

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    dec[b_off ^ 0] = static_cast<uint8_t>(state[r_off ^ 0] >> 8);
    dec[b_off ^ 1] = static_cast<uint8_t>(state[r_off ^ 0]);
    dec[b_off ^ 2] = static_cast<uint8_t>(state[r_off ^ 1] >> 8);
    dec[b_off ^ 3] = static_cast<uint8_t>(state[r_off ^ 1]);
  }
}
#else
static inline void
decrypt(const uint8_t* const __restrict inv_lut, // inverse look up table
        const uint8_t* const __restrict enc,     // input encrypted bytes
        uint8_t* const __restrict dec            // output decrypted bytes
)
{
  uint16_t state[8] = { 0u };

  constexpr size_t itr_cnt = harpocrates_common::N_ROWS >> 1;

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    state[r_off ^ 0] = (static_cast<uint16_t>(enc[b_off ^ 0]) << 8) |
                       static_cast<uint16_t>(enc[b_off ^ 1]);
    state[r_off ^ 1] = (static_cast<uint16_t>(enc[b_off ^ 2]) << 8) |
                       static_cast<uint16_t>(enc[b_off ^ 3]);
  }

  for (size_t i = 0; i < harpocrates_common::N_ROUNDS; i++) {
    using namespace harpocrates_utils;

    left_to_right_convoluted_substitution(state, inv_lut);
    column_substitution(state, inv_lut);
    add_rc(state, harpocrates_common::N_ROUNDS - (i + 1));
    right_to_left_convoluted_substitution(state, inv_lut);
  }

#if defined __clang__
#pragma unroll 4
#elif defined __GNUG__
#pragma GCC ivdep
#pragma GCC unroll 4
#endif
  for (size_t i = 0; i < itr_cnt; i++) {
    const size_t b_off = i << 2;
    const size_t r_off = i << 1;

    dec[b_off ^ 0] = static_cast<uint8_t>(state[r_off ^ 0] >> 8);
    dec[b_off ^ 1] = static_cast<uint8_t>(state[r_off ^ 0]);
    dec[b_off ^ 2] = static_cast<uint8_t>(state[r_off ^ 1] >> 8);
    dec[b_off ^ 3] = static_cast<uint8_t>(state[r_off ^ 1]);
  }
}
#endif

}
