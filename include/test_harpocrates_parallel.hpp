#pragma once
#include "harpocrates_parallel.hpp"
#include "utils.hpp"

// Test functional correctness of data-parallel Harpocrates cipher
// implementation
static void
test_harpocrates_parallel(sycl::queue& q,
                          const size_t wi_cnt,
                          const size_t wg_size)
{
  const size_t ct_len = wi_cnt << 4;

  // allocate resources
  uint8_t* lut = static_cast<uint8_t*>(sycl::malloc_device(256, q));
  uint8_t* ilut = static_cast<uint8_t*>(sycl::malloc_device(256, q));
  uint8_t* txt = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));
  uint8_t* enc = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));
  uint8_t* dec = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));

  // creation of (inverse) look up table is one time !
  //
  // lut  -> secret key when encrypting
  // ilut -> secret key when decrypting
  harpocrates_utils::generate_lut(lut);
  harpocrates_utils::generate_inv_lut(lut, ilut);

  random_data(txt, ct_len);

  using evt = sycl::event;
  using namespace harpocrates_parallel;

  // data-parallel encryption of a large byte array
  evt e0 = encrypt(q, lut, txt, enc, ct_len, wg_size, {});
  // data-parallel decryption of encrypted byte array
  evt e1 = decrypt(q, ilut, enc, dec, ct_len, wg_size, { e0 });

  // host synchronization
  e1.wait();

  // do byte-by-byte comparison of decrypted bytes against original input bytes
  for (size_t i = 0; i < ct_len; i++) {
    assert((txt[i] ^ dec[i]) == 0);
  }

  // release memory resources
  sycl::free(lut, q);
  sycl::free(ilut, q);
  sycl::free(txt, q);
  sycl::free(enc, q);
  sycl::free(dec, q);
}
