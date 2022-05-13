#include "test_harpocrates.hpp"
#include <bit>
#include <iostream>
#include <string.h>

// For testing data-parallel Harpocrates cipher implementation, using SYCL
#if defined TEST_SYCL
#include "test_harpocrates_parallel.hpp"
#endif

int
main()
{
  test_harpocrates_kat();
  std::cout << "[test] Harpocrates known answer tests ( KATs ) passed !"
            << std::endl;

  constexpr size_t itr_cnt = 1ul << 10;

  for (size_t i = 0; i < itr_cnt; i++) {
    test_harpocrates();
  }

  std::cout << "[test] Harpocrates random encrypt -> decrypt works !"
            << std::endl;

// For testing data-parallel Harpocrates cipher implementation, using SYCL
#if defined TEST_SYCL

#if defined TARGET_CPU
  sycl::cpu_selector s{};
#elif defined TARGET_GPU
  sycl::gpu_selector s{};
#else
  sycl::default_selector s{};
#endif

  sycl::device d{ s };
  sycl::context c{ d };
  sycl::queue q{ c, d };

  test_harpocrates_parallel(q, 1024ul, 32ul);
  std::cout << "[test] Data-parallel Harpocrates cipher works !" << std::endl;

#endif

  return EXIT_SUCCESS;
}
