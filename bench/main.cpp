#include "bench_harpocrates.hpp"

// register function for benchmarking
BENCHMARK(harpocrates_encrypt);
BENCHMARK(harpocrates_decrypt);

// main function to make it executable
BENCHMARK_MAIN();
