#pragma once
#include "harpocrates_parallel.hpp"
#include "utils.hpp"
#include <string.h>

constexpr double GB = 1073741824.; // 1 << 30 bytes
constexpr double MB = 1048576.;    // 1 << 20 bytes
constexpr double KB = 1024.;       // 1 << 10 bytes

// Data structure for holding
//
// i) host -> device input data transfer time
// ii) bytes of data transferred from host -> device
// iii) SYCL kernel execution time
// iv) device -> host output data transfer time
// v) bytes of data brought back to host from device
//
// where kernel in question can be Harpocrates encrypt/ decrypt routine,
// offloaded to accelerator device.
struct sycl_benchmark_t
{
  uint64_t h2d_tx_tm; // host -> device input data tx time ( ns )
  size_t h2d_tx;      // bytes of data transferred from host -> device ( bytes )
  uint64_t exec_tm;   // SYCL kernel execution time ( ns )
  uint64_t d2h_tx_tm; // device -> host output data tx time ( ns )
  size_t d2h_tx;      // bytes of data transferred from device -> host ( bytes )
};

// Time execution of SYCL command, whose submission resulted into supplied SYCL
// event
//
// Ensure where SYCL command was submitted, that queue has profiling enabled !
static inline uint64_t
time_event(sycl::event& e)
{
  using command = sycl::info::event_profiling;

  sycl::cl_ulong t0 = e.get_profiling_info<command::command_start>();
  sycl::cl_ulong t1 = e.get_profiling_info<command::command_end>();

  return static_cast<uint64_t>(t1 - t0);
}

// Convert how many bytes processed in how long timespan ( given in nanosecond
// level granularity ) to more human digestable format ( i.e. GB/ s or MB/ s or
// KB/ s or B/ s )
static inline const std::string
to_readable_bandwidth(const size_t bytes, // bytes
                      const uint64_t ts   // nanoseconds
)
{
  const double bytes_ = static_cast<double>(bytes);
  const double ts_ = static_cast<double>(ts) * 1e-9; // seconds
  const double bps = bytes_ / ts_;                   // bytes/ sec

  return bps >= GB   ? (std::to_string(bps / GB) + " GB/ s")
         : bps >= MB ? (std::to_string(bps / MB) + " MB/ s")
         : bps >= KB ? (std::to_string(bps / KB) + " KB/ s")
                     : (std::to_string(bps) + " B/ s");
}

// Convert given number of bytes to more readable form such as GB, MB, KB or B
static inline const std::string
to_readable_data_amount(const size_t bytes)
{
  const double bytes_ = static_cast<double>(bytes);

  return bytes_ >= GB   ? (std::to_string(bytes_ / GB) + " GB")
         : bytes_ >= MB ? (std::to_string(bytes_ / MB) + " MB")
         : bytes_ >= KB ? (std::to_string(bytes_ / KB) + " KB")
                        : (std::to_string(bytes_) + " B");
}

// Benchmark execution of data-parallel Harpocrates encrypt/ decrypt kernels,
// along with that keep track of host -> device & device -> host data transfer
// amount, time
//
// Note, in this routine, amount of data is calculated in bytes, while time is
// calculated in nanoseconds
//
// This function returns a vector of two elements, where first element holds
// benchmarked data for Harpocrates encrypt kernel, while second entry holds
// benchmarked data for Harpocrates decrypt kernel
static const std::vector<sycl_benchmark_t>
bench_harpocrates_parallel_encrypt_decrypt(
  sycl::queue& q,      // profiling enabled SYCL queue
  const size_t wi_cnt, // # -of work-items to dispatch
  const size_t wg_size // # -of work-items to group together
)
{
  // SYCL queue must have profiling enabled
  assert(q.has_property<sycl::property::queue::enable_profiling>());

  const size_t ct_len = wi_cnt << 4;

  // allocate resources
  uint8_t* lut_h = static_cast<uint8_t*>(sycl::malloc_host(256, q));
  uint8_t* ilut_h = static_cast<uint8_t*>(sycl::malloc_host(256, q));
  uint8_t* txt_h = static_cast<uint8_t*>(sycl::malloc_host(ct_len, q));
  uint8_t* enc_h = static_cast<uint8_t*>(sycl::malloc_host(ct_len, q));
  uint8_t* dec_h = static_cast<uint8_t*>(sycl::malloc_host(ct_len, q));

  uint8_t* lut_d = static_cast<uint8_t*>(sycl::malloc_device(256, q));
  uint8_t* ilut_d = static_cast<uint8_t*>(sycl::malloc_device(256, q));
  uint8_t* txt_d = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));
  uint8_t* enc_d = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));
  uint8_t* dec_d = static_cast<uint8_t*>(sycl::malloc_device(ct_len, q));

  harpocrates_utils::generate_lut(lut_h);
  harpocrates_utils::generate_inv_lut(lut_h, ilut_h);

  random_data(txt_h, ct_len);
  memset(enc_h, 0, ct_len);
  memset(dec_h, 0, ct_len);

  using evt = sycl::event;
  using evts = std::vector<evt>;
  using namespace harpocrates_parallel;

  // host -> device data tx
  evt e0 = q.memcpy(lut_d, lut_h, 256);
  evt e1 = q.memcpy(ilut_d, ilut_h, 256);
  evt e2 = q.memcpy(txt_d, txt_h, ct_len);

  evt e3 = q.memset(enc_d, 0, ct_len);
  evt e4 = q.memset(dec_d, 0, ct_len);

  // dispatch encryption kernel
  evts e5{ e0, e2, e3 };
  evt e6 = encrypt(q, lut_d, txt_d, enc_d, ct_len, wg_size, e5);

  // dispatch decryption kernel
  evts e7{ e1, e4, e6 };
  evt e8 = decrypt(q, ilut_d, enc_d, dec_d, ct_len, wg_size, e7);

  // device -> host data tx
  evt e9 = q.submit([&](sycl::handler& h) {
    h.depends_on(e6);
    h.memcpy(enc_h, enc_d, ct_len);
  });
  evt e10 = q.submit([&](sycl::handler& h) {
    h.depends_on(e8);
    h.memcpy(dec_h, dec_d, ct_len);
  });

  evt e11 = q.ext_oneapi_submit_barrier({ e9, e10 });

  // host sychronization
  e11.wait();

  // compare to check that encrypt -> decrypt on accelerator worked as expected
  for (size_t i = 0; i < ct_len; i++) {
    assert((txt_h[i] ^ dec_h[i]) == 0);
  }

  // release memory resources
  sycl::free(lut_h, q);
  sycl::free(ilut_h, q);
  sycl::free(txt_h, q);
  sycl::free(enc_h, q);
  sycl::free(dec_h, q);

  sycl::free(lut_d, q);
  sycl::free(ilut_d, q);
  sycl::free(txt_d, q);
  sycl::free(enc_d, q);
  sycl::free(dec_d, q);

  return {
    { time_event(e0) + time_event(e2),
      256ul + ct_len,
      time_event(e6),
      time_event(e9),
      ct_len }, // for encrypt kernel
    { time_event(e1) + time_event(e2),
      256ul + ct_len,
      time_event(e8),
      time_event(e10),
      ct_len } // for decrypt kernel
  };
}
