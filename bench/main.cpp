#include "harpocrates.hpp"
#include "utils.hpp"
#include <benchmark/benchmark.h>
#include <cassert>
#include <string.h>

// Benchmark Harpocrates single message block ( 16 -bytes ) encryption routine
// on CPU
static void
harpocrates_encrypt(benchmark::State& state)
{
  constexpr size_t lut_len = 256;
  constexpr size_t ct_len = harpocrates_common::BLOCK_LEN;

  uint8_t* lut = static_cast<uint8_t*>(std::malloc(lut_len));
  uint8_t* inv_lut = static_cast<uint8_t*>(std::malloc(lut_len));
  uint8_t* txt = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* enc = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* dec = static_cast<uint8_t*>(std::malloc(ct_len));

  harpocrates_utils::generate_lut(lut);
  harpocrates_utils::generate_inv_lut(lut, inv_lut);

  random_data(txt, ct_len);
  memset(enc, 0, ct_len);
  memset(dec, 0, ct_len);

  for (auto _ : state) {
    harpocrates::encrypt(lut, txt, enc);

    benchmark::DoNotOptimize(enc);
    benchmark::ClobberMemory();
  }

  harpocrates::decrypt(inv_lut, enc, dec);

  for (size_t i = 0; i < ct_len; i++) {
    assert((txt[i] ^ dec[i]) == 0);
  }

  const size_t total_data = ct_len * state.iterations();
  state.SetBytesProcessed(static_cast<int64_t>(total_data));

  std::free(lut);
  std::free(inv_lut);
  std::free(txt);
  std::free(enc);
  std::free(dec);
}

// Benchmark Harpocrates single message block ( 16 -bytes ) decryption routine
// on CPU
static void
harpocrates_decrypt(benchmark::State& state)
{
  constexpr size_t lut_len = 256;
  constexpr size_t ct_len = harpocrates_common::BLOCK_LEN;

  uint8_t* lut = static_cast<uint8_t*>(std::malloc(lut_len));
  uint8_t* inv_lut = static_cast<uint8_t*>(std::malloc(lut_len));
  uint8_t* txt = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* enc = static_cast<uint8_t*>(std::malloc(ct_len));
  uint8_t* dec = static_cast<uint8_t*>(std::malloc(ct_len));

  harpocrates_utils::generate_lut(lut);
  harpocrates_utils::generate_inv_lut(lut, inv_lut);

  random_data(txt, ct_len);
  memset(enc, 0, ct_len);
  memset(dec, 0, ct_len);

  harpocrates::encrypt(lut, txt, enc);

  for (auto _ : state) {
    harpocrates::decrypt(inv_lut, enc, dec);

    benchmark::DoNotOptimize(dec);
    benchmark::ClobberMemory();
  }

  for (size_t i = 0; i < ct_len; i++) {
    assert((txt[i] ^ dec[i]) == 0);
  }

  const size_t total_data = ct_len * state.iterations();
  state.SetBytesProcessed(static_cast<int64_t>(total_data));

  std::free(lut);
  std::free(inv_lut);
  std::free(txt);
  std::free(enc);
  std::free(dec);
}

// register function for benchmarking
BENCHMARK(harpocrates_encrypt);
BENCHMARK(harpocrates_decrypt);

// main function to make it executable
BENCHMARK_MAIN();
