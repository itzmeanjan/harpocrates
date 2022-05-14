#pragma once

#if !defined HARPOCRATES_PARALLEL
#define HARPOCRATES_PARALLEL
#endif

#include "harpocrates.hpp"
#include <CL/sycl.hpp>
#include <cassert>

// Data Parallel Harpocrates - An Efficient Parallel Encryption Mechanism for
// Data-at-rest, offloading to CPU & GPU using SYCL
//
// Read more about SYCL here
// https://www.khronos.org/registry/SYCL/specs/sycl-2020/html/sycl-2020.html
namespace harpocrates_parallel {

class kernelHarpocratesEncrypt;
class kernelHarpocratesDecrypt;

sycl::event
encrypt(sycl::queue&,
        const uint8_t* const __restrict,
        const uint8_t* const __restrict,
        uint8_t* const __restrict,
        const size_t,
        const size_t,
        std::vector<sycl::event>);

sycl::event
decrypt(sycl::queue&,
        const uint8_t* const __restrict,
        const uint8_t* const __restrict,
        uint8_t* const __restrict,
        const size_t,
        const size_t,
        std::vector<sycl::event>);

// Given 256 -bytes look up table & N -bytes plain text, this routine offloads
// computation of encrypted byte slices to available accelerator device (
// encapsulated in SYCL queue object ), producing N -many encrypted bytes.
//
// For encrypting each 16 -bytes slice, this routine makes use of Harpocrates
// cipher. Which is exactly why N must be evenly divisible by 16.
//
// Also note, dispatched work group size must be evenly dividing `ct_len >> 4`,
// so that each work-group has same number of active work-items
//
// For creating SYCL dependency graph, make use of last parameter & return type
// of this routine.
sycl::event
encrypt(sycl::queue& q,                      // SYCL queue
        const uint8_t* const __restrict lut, // 256 -bytes look up table
        const uint8_t* const __restrict txt, // plain text | len(txt) = N
        uint8_t* const __restrict enc,       // encrypted text | len(enc) = N
        const size_t ct_len,                 // len(txt) = len(enc) = N
        const size_t wg_size,                // (ct_len >> 4) % wg_size == 0
        std::vector<sycl::event> evts        // SYCL events to wait for
)
{
  // ensure input plain text is of non-zero length
  assert(ct_len > 0ul);
  // ensure input plain text length is evenly divisible by 16,
  // so that we can dispatch `ct_len >> 4` -many work-items
  assert((ct_len & 15ul) == 0ul);

  // these many work-items to be dispatched so that each work-item
  // can encrypt its portion of plain text independently
  const size_t wi_cnt = ct_len >> 4;

  // all work-groups must have same #- of active work-items
  assert(wi_cnt % wg_size == 0);

  return q.submit([&](sycl::handler& h) {
    // create dependency graph
    h.depends_on(evts);

    sycl::accessor<uint8_t,
                   1,
                   sycl::access_mode::read_write,
                   sycl::access::target::local>
      loc_lut{ sycl::range<1>{ 256ul }, h };

    const auto rng = sycl::nd_range<1>{ wi_cnt, wg_size };
    h.parallel_for<kernelHarpocratesEncrypt>(rng, [=](sycl::nd_item<1> it) {
      const auto grp = it.get_group();

      if (it.get_local_linear_id() == 0ul) {
        for (size_t i = 0; i < 256ul; i++) {
          loc_lut[i] = lut[i];
        }
      }

      sycl::group_barrier(grp, sycl::memory_scope_work_group);

      const size_t idx = it.get_global_linear_id();
      const size_t b_off = idx << 4;

      harpocrates::encrypt(loc_lut.get_pointer(), txt + b_off, enc + b_off);
    });
  });
}

// Given 256 -bytes inverse look up table & N -bytes cipher text, this routine
// offloads computation of decrypted byte slices to available accelerator device
// ( encapsulated in SYCL queue object ), producing N -many decrypted bytes.
//
// For decrypting each 16 -bytes slice, this routine makes use of Harpocrates
// cipher. Which is exactly why N must be evenly divisible by 16.
//
// Also note, dispatched work group size must be evenly dividing `ct_len >> 4`,
// so that each work-group has same number of active work-items
//
// For creating SYCL dependency graph, make use of last parameter & return type
// of this routine.
sycl::event
decrypt(
  sycl::queue& q,                          // SYCL queue
  const uint8_t* const __restrict inv_lut, // 256 -bytes inverse look up table
  const uint8_t* const __restrict enc,     // encrypted bytes | len(enc) = N
  uint8_t* const __restrict dec,           // decrypted bytes | len(dec) = N
  const size_t ct_len,                     // len(enc) = len(dec) = N
  const size_t wg_size,                    // (ct_len >> 4) % wg_size == 0
  std::vector<sycl::event> evts            // SYCL events to wait for
)
{
  // ensure input cipher text is of non-zero length
  assert(ct_len > 0ul);
  // ensure input cipher text length is evenly divisible by 16,
  // so that we can dispatch `ct_len >> 4` -many work-items
  assert((ct_len & 15ul) == 0ul);

  // these many work-items to be dispatched so that each work-item
  // can decrypt its portion of cipher text independently
  const size_t wi_cnt = ct_len >> 4;

  // all work-groups must have same #- of active work-items
  assert(wi_cnt % wg_size == 0);

  return q.submit([&](sycl::handler& h) {
    // create dependency graph
    h.depends_on(evts);

    sycl::accessor<uint8_t,
                   1,
                   sycl::access_mode::read_write,
                   sycl::access::target::local>
      loc_inv_lut{ sycl::range<1>{ 256ul }, h };

    const auto rng = sycl::nd_range<1>{ wi_cnt, wg_size };
    h.parallel_for<kernelHarpocratesDecrypt>(rng, [=](sycl::nd_item<1> it) {
      const auto grp = it.get_group();

      if (it.get_local_linear_id() == 0ul) {
        for (size_t i = 0; i < 256ul; i++) {
          loc_inv_lut[i] = inv_lut[i];
        }
      }

      sycl::group_barrier(grp, sycl::memory_scope_work_group);

      const size_t idx = it.get_global_linear_id();
      const size_t b_off = idx << 4;

      harpocrates::decrypt(loc_inv_lut.get_pointer(), enc + b_off, dec + b_off);
    });
  });
}

}
