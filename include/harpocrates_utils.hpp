#pragma once
#include <cstdint>
#include <random>

using size_t = std::size_t;

// Harpocartes - An Efficient Encryption Mechanism for Data-at-rest, related
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

  constexpr size_t n = 256;
  for (size_t i = 0; i < n - 1; i++) {
    std::uniform_int_distribution<uint8_t> dis{ i, n - 1ul };
    const uint8_t j = dis(gen);

    lut[i] ^= lut[j];
    lut[j] ^= lut[i];
    lut[i] ^= lut[j];
  }
}

}
