CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic
OPTFLAGS = -O3
IFLAGS = -I ./include

DTARGET = -DTARGET_$(shell echo $(or $(TARGET),default) | tr a-z A-Z)

all: test_harpocrates

test/harpocrates.out: test/main.cpp include/*.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(IFLAGS) $< -o $@

test_harpocrates: test/harpocrates.out
	./$<

clean:
	find . -name '*.out' -o -name '*.o' -o -name '*.so' -o -name '*.gch' | xargs rm -rf

format:
	find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i --style=Mozilla

lib:
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(IFLAGS) -fPIC --shared wrapper/harpocrates.cpp -o wrapper/libharpocrates.so

test_rust:
	make lib && cd wrapper/rust && LD_LIBRARY_PATH=.. cargo test --lib && cd ../..

bench_rust:
	make lib && cd wrapper/rust && LD_LIBRARY_PATH=.. cargo criterion --output-format verbose && cd ../..

# benchmarks Harpocrates minimal cipher variant on CPU
bench/harpocrates.out: bench/harpocrates.cpp include/*.hpp
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(IFLAGS) $< -lbenchmark -o $@

benchmark: bench/harpocrates.out
	./$<

# tests data-parallel version of Harpocrates kernels
test/harpocrates_parallel.out: test/main.cpp include/*.hpp
	dpcpp $(CXXFLAGS) -DTEST_SYCL -fsycl $(DTARGET) $(OPTFLAGS) $(IFLAGS) $< -o $@

test_harpocrates_parallel: test/harpocrates_parallel.out
	./$<

# compiles Harpocrates kernels on-the-fly & benchmarks on selected ( or available )
# accelerator device
bench/harpocrates_parallel.out: bench/harpocrates_parallel.cpp include/*.hpp
	dpcpp $(CXXFLAGS) -fsycl $(DTARGET) $(OPTFLAGS) $(IFLAGS) $< -o $@

benchmark_parallel: bench/harpocrates_parallel.out
	./$<

# attempt to AOT compile both kernels, when targeting Intel CPUs,
# using AVX/ AVX2 /AVX512/ SSE4.2 ( when available )
bench/aot_cpu.out: bench/harpocrates_parallel.cpp include/*.hpp
	@if lscpu | grep -q 'avx512'; then \
		echo "Using avx512"; \
		dpcpp $(CXXFLAGS) -fsycl -fsycl-targets=spir64_x86_64 -Xs "-march=avx512" -DTARGET_CPU $(OPTFLAGS) $(IFLAGS) $< -o $@; \
	elif lscpu | grep -q 'avx2'; then \
		echo "Using avx2"; \
		dpcpp $(CXXFLAGS) -fsycl -fsycl-targets=spir64_x86_64 -Xs "-march=avx2" -DTARGET_CPU $(OPTFLAGS) $(IFLAGS) $< -o $@; \
	elif lscpu | grep -q 'avx'; then \
		echo "Using avx"; \
		dpcpp $(CXXFLAGS) -fsycl -fsycl-targets=spir64_x86_64 -Xs "-march=avx" -DTARGET_CPU $(OPTFLAGS) $(IFLAGS) $< -o $@; \
	elif lscpu | grep -q 'sse4.2'; then \
		echo "Using sse4.2"; \
		dpcpp $(CXXFLAGS) -fsycl -fsycl-targets=spir64_x86_64 -Xs "-march=sse4.2" -DTARGET_CPU $(OPTFLAGS) $(IFLAGS) $< -o $@; \
	else \
		echo "Can't AOT compile using avx, avx2, avx512 or sse4.2"; \
	fi

aot_cpu: bench/aot_cpu.out
	./$<

# attempt to AOT compile both kernels, when targeting Intel GPUs
bench/aot_gpu.out: bench/harpocrates_parallel.cpp include/*.hpp
	# you may want to replace `device` identifier with `0x3e96` if you're targeting *Intel(R) UHD Graphics P630*
	#
	# otherwise, let it be what it's when you're targeting *Intel(R) Iris(R) Xe MAX Graphics*
	dpcpp $(CXXFLAGS) -fsycl -fsycl-targets=spir64_gen -Xs "-device 0x4905" -DTARGET_GPU $(OPTFLAGS) $(IFLAGS) $< -o $@

aot_gpu: bench/aot_gpu.out
	./$<

# target CUDA for benchmarking data-parallel Harpocrates cipher
bench/harpocrates_parallel.cuda.out: bench/harpocrates_parallel.cpp include/*.hpp
	clang++ $(CXXFLAGS) -fsycl -fsycl-targets=nvptx64-nvidia-cuda -DTARGET_GPU $(OPTFLAGS) $(IFLAGS) $< -o $@

cuda: bench/harpocrates_parallel.cuda.out
	./$<
