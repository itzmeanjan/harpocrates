#include "harpocrates.hpp"

// Thin C wrapper on top of underlying C++ implementation of Harpocrates Cipher,
// which can be used for producing shared library object with C-ABI & used from
// other languages such as Rust, Python

// Function declarations
extern "C"
{
  void encrypt(const uint8_t* const __restrict,
               const uint8_t* const __restrict,
               uint8_t* const __restrict);

  void decrypt(const uint8_t* const __restrict,
               const uint8_t* const __restrict,
               uint8_t* const __restrict);
}

// Function implementations
extern "C"
{
  // Given 256 -bytes look up table, 16 -bytes plain text, this routine computes
  // 16 -bytes encrypted data using Harpocrates encryption algorithm
  //
  // Input:
  //
  // - lut: 256 -bytes of look up table i.e. len(lut) == 256
  // - txt: 16 -bytes plain text i.e. len(txt) == 16
  //
  // Output:
  //
  // - enc: 16 -bytes of encrypted data i.e. len(enc) == 16
  void encrypt(const uint8_t* const __restrict lut,
               const uint8_t* const __restrict txt,
               uint8_t* const __restrict enc)
  {

    harpocrates::encrypt(lut, txt, enc);
  }

  // Given 256 -bytes inverse look up table, 16 -bytes encrypted data, this
  // routine computes 16 -bytes decrypted data using Harpocrates decryption
  // algorithm
  //
  // Input:
  //
  // - ilut: 256 -bytes of inverse look up table i.e. len(ilut) == 256
  // - enc: 16 -bytes encrypted text i.e. len(enc) == 16
  //
  // Output:
  //
  // - dec: 16 -bytes of decrypted data i.e. len(dec) == 16
  void decrypt(const uint8_t* const __restrict ilut,
               const uint8_t* const __restrict enc,
               uint8_t* const __restrict dec)
  {
    harpocrates::decrypt(ilut, enc, dec);
  }
}
