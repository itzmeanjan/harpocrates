# harpocrates
Harpocrates - An Efficient Encryption for Data-at-rest

## Overview

`harpocrates` is an implementation of an efficient algorithm for encrypting data-at-rest. Each message block ( 16 -bytes ) can be independently encrypted/ decrypted, given that (inverse) look up table ( read LUT ) has already been generated. State size of this lightweight cipher is only 128 -bit & it's powered by substitution convolution network ( read SCN ). Only substitution, bit shifting, bit rotation & XOR are used as primitive operations.

> Note, (inv) LUT generation is one-time operation.

Here I'm keeping a zero-dependency, easy-to-use C++ header-only library ( using C++20 features ), implementing Harpocrates specification, as described [here](https://ia.cr/2022/519). This implementation can be compiled targeting both CPUs, GPUs ( using SYCL ).

## Prerequisites

- Ensure you've C++ compiler such as `g++`/ `clang++`, along with C++20 standard library

> I'm using

```bash
$ g++ --version
g++ (Ubuntu 11.2.0-19ubuntu1) 11.2.0
```

- You'll also need to have standard system development utilities such as `make`/ `cmake`

> I'm using

```bash
$ make --version
GNU Make 4.3

$ cmake --version
cmake version 3.22.1
```

- For benchmarking Harpocrates implementation on CPU, you need to have `google-benchmark` library globally installed; see [this](https://github.com/google/benchmark/tree/60b16f1#installation)

## Testing

For testing functional correctness of Harpocrates cipher implementation, issue following command, which runs two kinds of tests

- Asserting results against Known Answer Tests ( read KATs ) supplied with Harpocrates specification **[ Correctness & Conformance ]**
- With randomly generated message blocks, attempt to execute encrypt -> decrypt cycle **[ Correctness ]**

```bash
make
```

## Benchmarking

For benchmarking Harpocrates cipher implementation, using single message block ( 16 -bytes ), on CPU, issue

```bash
make benchmark
```

### ARM Cortex-A72

```bash
022-05-11T16:31:36+05:30
Running ./bench/a.out
Run on (4 X 1800 MHz CPU s)
Load Average: 0.74, 1.20, 1.45
------------------------------------------------------------------------------
Benchmark                    Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------
harpocrates_encrypt       3443 ns         3443 ns       203330 bytes_per_second=4.43226M/s
harpocrates_decrypt       3461 ns         3461 ns       202333 bytes_per_second=4.40924M/s
```

## Usage

`harpocrates` being C++ header-only library, using it's as easy as including `./include/harpocrates.hpp` in your program, while asking your compiler to include `./include` in its INCLUDE_PATH ( using `-I` flag ).

- Ideally you'd want to use `harpocrates_utils::` namespace for generating (inv)LUT, which is one-time process ( in pre-compute phase )
- After that you'll only need `harpocrates::` namespace, which implements `encrypt`/ `decrypt` routines
- You may also want to see `harpocrates_common::` namespace, which defines some constants

I've kept `harpocrates` API usage example [here](https://github.com/itzmeanjan/harpocrates/blob/9c1233d/example/main.cpp).
