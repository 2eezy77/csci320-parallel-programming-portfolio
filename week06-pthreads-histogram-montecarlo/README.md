# Week 6: Histogram and Monte Carlo π (Pthreads)

Jose I. Montero — CSCI 320 Parallel & Distributed Programming

## Contents

- `pthreads_histogram.c` — Pthreads histogram
- `pthreads_monte_carlo_pi.c` — Pthreads Monte Carlo π estimator
- `Makefile` — builds both programs (`gcc -pthread`)
- `sample_data.txt` — tiny test input for the histogram

## Build

```bash
make all
```

## Clean

```bash
make clean
```

## Run examples

```bash
./pthreads_histogram 5 0.0 10.0 sample_data.txt 4        # bins min max file threads
./pthreads_monte_carlo_pi 5000000 4                      # tosses threads
```

## Implementation notes

- **Histogram:** main thread reads all values, slices work by index range, each thread owns a private bin array, then main reduces after join (no locks needed). Values outside `[min_val, max_val]` are ignored; `x == max_val` falls into the last bin. Uses `long long` bin counts.
- **Monte Carlo π:** static work partition by toss count; each thread gets its own xorshift64* RNG stream; π = `4 * hits / tosses` using `long long` counts.

## Sample output

```text
./pthreads_histogram 5 0.0 10.0 sample_data.txt 4
Histogram (bin edges [0, 10], 5 bins, 4 threads):
[0, 2) : 3
[2, 4) : 4
[4, 6) : 4
[6, 8) : 4
[8, 10)] : 5

./pthreads_monte_carlo_pi 5000000 4
total_tosses=5000000  total_in_circle=3927176  pi_estimate=3.14174080  threads=4
```
