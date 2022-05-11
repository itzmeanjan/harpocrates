#include "test_harpocrates.hpp"
#include <bit>
#include <iostream>
#include <string.h>

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

  return EXIT_SUCCESS;
}
