#pragma once
#include <cstdint>
#include <random>

using size_t = std::size_t;

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

  constexpr uint8_t n = 255;
  for (uint8_t i = 0; i < n; i++) {
    std::uniform_int_distribution<uint8_t> dis{ i, n };
    const uint8_t j = dis(gen);

    lut[i] ^= lut[j];
    lut[j] ^= lut[i];
    lut[i] ^= lut[j];
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

}
