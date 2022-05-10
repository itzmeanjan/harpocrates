#include "test_harpocrates.hpp"
#include <iostream>
#include <string.h>

int
main()
{
  constexpr size_t itr_cnt = 1ul << 10;

  for (size_t i = 0; i < itr_cnt; i++) {
    test_harpocrates();
  }

  std::cout << "[test] Harpocrates cipher works !" << std::endl;

  return EXIT_SUCCESS;
}
