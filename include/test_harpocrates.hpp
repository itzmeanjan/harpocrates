#pragma once
#include "harpocrates.hpp"
#include "utils.hpp"
#include <cassert>

// Tests functional correctness of Harpocrates cipher implementation
static inline void
test_harpocrates()
{
  constexpr size_t ct_len = harpocrates_common::BLOCK_LEN;

  // acquire memory resources
  uint8_t* lut = static_cast<uint8_t*>(std::malloc(256));
  uint8_t* inv_lut = static_cast<uint8_t*>(std::malloc(256));
  uint8_t* txt = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* enc = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* dec = static_cast<uint8_t*>(std::malloc(ct_len));

  harpocrates_utils::generate_lut(lut);              // used for encryption
  harpocrates_utils::generate_inv_lut(lut, inv_lut); // used for decryption

  random_data(txt, ct_len); // random plain text

  harpocrates::encrypt(lut, txt, enc);     // encrypt a block
  harpocrates::decrypt(inv_lut, enc, dec); // decrypt a block

  // check that decryption worked as expected
  for (size_t i = 0; i < ct_len; i++) {
    assert((txt[i] ^ dec[i]) == 0u);
  }

  // deallocate all resources
  std::free(lut);
  std::free(inv_lut);
  std::free(txt);
  std::free(enc);
  std::free(dec);
}
