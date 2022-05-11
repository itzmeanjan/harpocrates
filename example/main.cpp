#include "harpocrates.hpp"
#include "utils.hpp"
#include <cassert>
#include <iostream>
#include <string.h>

// Compile it with
// g++ -std=c++20 -Wall -Wextra -pedantic -O3 -I ./include example/main.cpp
int
main()
{
  uint8_t lut[256], inv_lut[256];
  uint8_t txt[16], enc[16], dec[16];

  random_data(txt, 16);

  memset(enc, 0, 16);
  memset(dec, 0, 16);

  // one-time compute
  harpocrates_utils::generate_lut(lut);
  harpocrates_utils::generate_inv_lut(lut, inv_lut);

  harpocrates::encrypt(lut, txt, enc);
  harpocrates::decrypt(inv_lut, enc, dec);

  for (size_t i = 0; i < 16; i++) {
    assert((txt[i] ^ dec[i]) == 0);
  }

  std::cout << "Plain Text : " << to_hex(txt, 16) << std::endl;
  std::cout << "Encrypted  : " << to_hex(enc, 16) << std::endl;
  std::cout << "Decrypted  : " << to_hex(dec, 16) << std::endl;

  return EXIT_SUCCESS;
}
