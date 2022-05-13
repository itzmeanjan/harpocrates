#include "bench_harpocrates_parallel.hpp"
#include "table.hpp"

int
main()
{
#if defined TARGET_CPU
  sycl::cpu_selector s{};
#pragma message("Targeting default CPU at run-time")
#elif defined TARGET_GPU
  sycl::gpu_selector s{};
#pragma message("Targeting default GPU at run-time")
#else
  sycl::default_selector s{};
#pragma message("Targeting default Accelerator at run-time")
#endif

  sycl::device d{ s };
  sycl::context c{ d };
  sycl::queue q{ c, d, sycl::property::queue::enable_profiling{} };

  constexpr size_t min_wi_cnt = 1ul << 20;
  constexpr size_t max_wi_cnt = 1ul << 26;
  constexpr size_t wg_size = 32ul;

  std::cout << "Running on " << d.get_info<sycl::info::device::name>()
            << std::endl
            << std::endl;

  TextTable tbl('-', '|', '+');

  tbl.add("# -of work-items");
  tbl.add("kernel name");
  tbl.add("input size ( bytes )");
  tbl.add("output size ( bytes )");
  tbl.add("host-to-device b/w");
  tbl.add("kernel b/w");
  tbl.add("device-to-host b/w");
  tbl.endOfRow();

  for (size_t wi = min_wi_cnt; wi <= max_wi_cnt; wi <<= 1) {
    const auto ret = bench_harpocrates_parallel_encrypt_decrypt(q, wi, wg_size);

    const auto enc_kernel = ret[0];
    const auto dec_kernel = ret[1];

    tbl.add(std::to_string(wi));
    tbl.add("Harpocrates Encrypt");
    tbl.add(to_readable_data_amount(enc_kernel.h2d_tx));
    tbl.add(to_readable_data_amount(enc_kernel.d2h_tx));
    tbl.add(to_readable_bandwidth(enc_kernel.h2d_tx, enc_kernel.h2d_tx_tm));
    tbl.add(to_readable_bandwidth(enc_kernel.h2d_tx - 256, enc_kernel.exec_tm));
    tbl.add(to_readable_bandwidth(enc_kernel.d2h_tx, enc_kernel.d2h_tx_tm));
    tbl.endOfRow();

    tbl.add(std::to_string(wi));
    tbl.add("Harpocrates Decrypt");
    tbl.add(to_readable_data_amount(dec_kernel.h2d_tx));
    tbl.add(to_readable_data_amount(dec_kernel.d2h_tx));
    tbl.add(to_readable_bandwidth(dec_kernel.h2d_tx, dec_kernel.h2d_tx_tm));
    tbl.add(to_readable_bandwidth(dec_kernel.h2d_tx - 256, dec_kernel.exec_tm));
    tbl.add(to_readable_bandwidth(dec_kernel.d2h_tx, dec_kernel.d2h_tx_tm));
    tbl.endOfRow();
  }

  tbl.setAlignment(1, TextTable::Alignment::RIGHT);
  tbl.setAlignment(2, TextTable::Alignment::RIGHT);
  tbl.setAlignment(3, TextTable::Alignment::RIGHT);
  tbl.setAlignment(4, TextTable::Alignment::RIGHT);
  tbl.setAlignment(5, TextTable::Alignment::RIGHT);
  tbl.setAlignment(6, TextTable::Alignment::RIGHT);

  std::cout << tbl;

  return EXIT_SUCCESS;
}
