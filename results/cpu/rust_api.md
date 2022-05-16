# Benchmarking Rust API of Harpocrates Cipher on ARM Cortex-A72

Compile, link & execute benchmark using cargo-criterion

```bash
# assuming you've already installed cargo-criterion, if not
#
# cargo install cargo-criterion

make bench_rust
```

```bash
$ lscpu | grep -i cpu\(s\) # available number of CPU cores

CPU(s):                          16
On-line CPU(s) list:             0-15
NUMA node0 CPU(s):               0-15
```

## Basic Harpocrates Encrypt/ Decrypt Rust API

```bash
harpocrates/encrypt     time:   [1.8458 us 1.8458 us 1.8459 us]
                        thrpt:  [8.2664 MiB/s 8.2667 MiB/s 8.2670 MiB/s]
                 change:
                        time:   [-0.0120% +0.0009% +0.0166%] (p = 0.92 > 0.05)
                        thrpt:  [-0.0165% -0.0009% +0.0120%]
                        No change in performance detected.
Found 8 outliers among 100 measurements (8.00%)
  1 (1.00%) low severe
  1 (1.00%) low mild
  3 (3.00%) high mild
  3 (3.00%) high severe
slope  [1.8458 us 1.8459 us] R^2            [0.9999967 0.9999966]
mean   [1.8458 us 1.8462 us] std. dev.      [243.42 ps 2.2510 ns]
median [1.8457 us 1.8458 us] med. abs. dev. [92.281 ps 208.90 ps]

harpocrates/decrypt     time:   [1.8294 us 1.8295 us 1.8296 us]
                        thrpt:  [8.3402 MiB/s 8.3406 MiB/s 8.3410 MiB/s]
                 change:
                        time:   [-0.1231% -0.0339% +0.0164%] (p = 0.62 > 0.05)
                        thrpt:  [-0.0163% +0.0340% +0.1232%]
                        No change in performance detected.
Found 15 outliers among 100 measurements (15.00%)
  1 (1.00%) low severe
  9 (9.00%) high mild
  5 (5.00%) high severe
slope  [1.8294 us 1.8296 us] R^2            [0.9999939 0.9999938]
mean   [1.8294 us 1.8297 us] std. dev.      [432.77 ps 985.24 ps]
median [1.8293 us 1.8294 us] med. abs. dev. [137.70 ps 281.54 ps]
```

## Sequential Harpocrates Rust API for Encrypting/ Decrypting Large Byte Arrays

Benchmarks routine which encrypts/ decrypts N -many byte chunks sequentially ( one after another ) | total input/ output size = (N << 4) -bytes ∈ {16MB, 32MB, 64MB}

```bash
harpocrates_with_data_chunks/encrypt/16 MB
                        time:   [1.9323 s 1.9323 s 1.9324 s]
                        thrpt:  [8.2799 MiB/s 8.2801 MiB/s 8.2802 MiB/s]
                 change:
                        time:   [+0.0050% +0.0070% +0.0090%] (p = 0.00 < 0.05)
                        thrpt:  [-0.0090% -0.0070% -0.0050%]
                        Change within noise threshold.
mean   [1.9323 s 1.9324 s] std. dev.      [28.010 us 69.871 us]
median [1.9323 s 1.9324 s] med. abs. dev. [2.3410 us 108.39 us]

harpocrates_with_data_chunks/decrypt/16 MB
                        time:   [1.9125 s 1.9126 s 1.9127 s]
                        thrpt:  [8.3653 MiB/s 8.3657 MiB/s 8.3661 MiB/s]
                 change:
                        time:   [+0.0036% +0.0083% +0.0134%] (p = 0.01 < 0.05)
                        thrpt:  [-0.0134% -0.0083% -0.0036%]
                        Change within noise threshold.
mean   [1.9125 s 1.9127 s] std. dev.      [93.212 us 187.07 us]
median [1.9124 s 1.9127 s] med. abs. dev. [10.111 us 250.81 us]

harpocrates_with_data_chunks/encrypt/32 MB
                        time:   [3.8643 s 3.8644 s 3.8644 s]
                        thrpt:  [8.2807 MiB/s 8.2808 MiB/s 8.2809 MiB/s]
                 change:
                        time:   [-0.0056% -0.0036% -0.0019%] (p = 0.00 < 0.05)
                        thrpt:  [+0.0019% +0.0036% +0.0056%]
                        Change within noise threshold.
Found 2 outliers among 10 measurements (20.00%)
  2 (20.00%) low mild
mean   [3.8643 s 3.8644 s] std. dev.      [32.688 us 111.53 us]
median [3.8643 s 3.8644 s] med. abs. dev. [4.8733 us 147.82 us]

harpocrates_with_data_chunks/decrypt/32 MB
                        time:   [3.8250 s 3.8250 s 3.8250 s]
                        thrpt:  [8.3659 MiB/s 8.3660 MiB/s 8.3661 MiB/s]
                 change:
                        time:   [-0.0004% +0.0006% +0.0019%] (p = 0.34 > 0.05)
                        thrpt:  [-0.0019% -0.0006% +0.0004%]
                        No change in performance detected.
Found 1 outliers among 10 measurements (10.00%)
  1 (10.00%) high mild
mean   [3.8250 s 3.8250 s] std. dev.      [34.369 us 92.670 us]
median [3.8249 s 3.8250 s] med. abs. dev. [6.6658 us 119.06 us]

harpocrates_with_data_chunks/encrypt/64 MB
                        time:   [7.7289 s 7.7294 s 7.7304 s]
                        thrpt:  [8.2790 MiB/s 8.2801 MiB/s 8.2806 MiB/s]
                 change:
                        time:   [-0.0025% +0.0048% +0.0181%] (p = 0.66 > 0.05)
                        thrpt:  [-0.0181% -0.0048% +0.0025%]
                        No change in performance detected.
Found 2 outliers among 10 measurements (20.00%)
  1 (10.00%) high mild
  1 (10.00%) high severe
mean   [7.7289 s 7.7304 s] std. dev.      [45.321 us 2.4537 ms]
median [7.7289 s 7.7290 s] med. abs. dev. [24.053 us 207.60 us]

harpocrates_with_data_chunks/decrypt/64 MB
                        time:   [7.6499 s 7.6499 s 7.6500 s]
                        thrpt:  [8.3660 MiB/s 8.3661 MiB/s 8.3661 MiB/s]
                 change:
                        time:   [-0.0052% -0.0041% -0.0031%] (p = 0.00 < 0.05)
                        thrpt:  [+0.0031% +0.0041% +0.0052%]
                        Change within noise threshold.
mean   [7.6499 s 7.6500 s] std. dev.      [50.185 us 104.79 us]
median [7.6499 s 7.6500 s] med. abs. dev. [7.4293 us 147.40 us]
```

## Parallel Harpocrates Rust API for Encrypting/ Decrypting Large Byte Arrays

Benchmarks routine which parallelly encrypts/ decrypts N -many byte chunks ( spreading computation over all available CPU cores ) | total input/ output size = (N << 4) -bytes ∈ {16MB, 32MB, 64MB}

```bash
par_harpocrates_with_data_chunks/encrypt/16 MB
                        time:   [122.05 ms 122.53 ms 122.86 ms]
                        thrpt:  [130.23 MiB/s 130.58 MiB/s 131.09 MiB/s]
                 change:
                        time:   [-0.6625% -0.1642% +0.3855%] (p = 0.56 > 0.05)
                        thrpt:  [-0.3841% +0.1645% +0.6669%]
                        No change in performance detected.
slope  [122.05 ms 122.86 ms] R^2            [0.9993045 0.9994527]
mean   [121.73 ms 122.70 ms] std. dev.      [471.65 us 1.0510 ms]
median [121.33 ms 122.79 ms] med. abs. dev. [117.37 us 1.3988 ms]

par_harpocrates_with_data_chunks/decrypt/16 MB
                        time:   [120.74 ms 121.10 ms 121.70 ms]
                        thrpt:  [131.47 MiB/s 132.12 MiB/s 132.51 MiB/s]
                 change:
                        time:   [-1.0900% -0.1300% +0.7273%] (p = 0.80 > 0.05)
                        thrpt:  [-0.7220% +0.1302% +1.1020%]
                        No change in performance detected.
Found 1 outliers among 10 measurements (10.00%)
  1 (10.00%) high mild
slope  [120.74 ms 121.70 ms] R^2            [0.9992336 0.9989205]
mean   [120.74 ms 122.04 ms] std. dev.      [368.45 us 1.4857 ms]
median [120.65 ms 122.02 ms] med. abs. dev. [128.34 us 1.6619 ms]

par_harpocrates_with_data_chunks/encrypt/32 MB
                        time:   [243.74 ms 244.68 ms 246.11 ms]
                        thrpt:  [130.02 MiB/s 130.79 MiB/s 131.29 MiB/s]
                 change:
                        time:   [-0.3333% +0.2045% +0.8007%] (p = 0.58 > 0.05)
                        thrpt:  [-0.7943% -0.2041% +0.3344%]
                        No change in performance detected.
Found 1 outliers among 10 measurements (10.00%)
  1 (10.00%) high severe
mean   [243.74 ms 246.11 ms] std. dev.      [495.12 us 3.2186 ms]
median [243.45 ms 244.82 ms] med. abs. dev. [191.67 us 1.7622 ms]

par_harpocrates_with_data_chunks/decrypt/32 MB
                        time:   [241.04 ms 241.43 ms 241.89 ms]
                        thrpt:  [132.29 MiB/s 132.54 MiB/s 132.76 MiB/s]
                 change:
                        time:   [-0.3068% -0.0047% +0.3188%] (p = 0.98 > 0.05)
                        thrpt:  [-0.3178% +0.0047% +0.3078%]
                        No change in performance detected.
Found 1 outliers among 10 measurements (10.00%)
  1 (10.00%) high mild
mean   [241.04 ms 241.89 ms] std. dev.      [327.64 us 999.79 us]
median [240.82 ms 241.86 ms] med. abs. dev. [135.53 us 1.1338 ms]

par_harpocrates_with_data_chunks/encrypt/64 MB
                        time:   [486.95 ms 487.42 ms 487.92 ms]
                        thrpt:  [131.17 MiB/s 131.30 MiB/s 131.43 MiB/s]
                 change:
                        time:   [-0.1703% -0.0079% +0.1559%] (p = 0.92 > 0.05)
                        thrpt:  [-0.1557% +0.0079% +0.1706%]
                        No change in performance detected.
mean   [486.95 ms 487.92 ms] std. dev.      [448.71 us 1.0093 ms]
median [486.65 ms 488.06 ms] med. abs. dev. [105.41 us 1.4396 ms]

par_harpocrates_with_data_chunks/decrypt/64 MB
                        time:   [481.38 ms 481.88 ms 482.36 ms]
                        thrpt:  [132.68 MiB/s 132.81 MiB/s 132.95 MiB/s]
                 change:
                        time:   [-0.1653% -0.0188% +0.1279%] (p = 0.81 > 0.05)
                        thrpt:  [-0.1278% +0.0188% +0.1656%]
                        No change in performance detected.
mean   [481.38 ms 482.36 ms] std. dev.      [486.98 us 1.0680 ms]
median [481.18 ms 482.49 ms] med. abs. dev. [14.421 us 1.3692 ms]
```
