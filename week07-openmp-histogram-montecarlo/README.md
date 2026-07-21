# Week 7: Histogram and Monte Carlo π (OpenMP)

Jose I. Montero — CSCI 320 Parallel & Distributed Programming

## Contents

- `openmp_histogram.c` — OpenMP histogram
- `openmp_monte_carlo_pi.c` — OpenMP Monte Carlo π estimator
- `Makefile` — builds both programs (`gcc -fopenmp`)
- `sample_data.txt` — tiny test input for the histogram

## Build

```bash
make
```

## Run examples

```bash
./openmp_histogram 5 0.0 10.0 sample_data.txt            # bins min max file [threads]
./openmp_histogram 5 0.0 10.0 sample_data.txt 4          # with explicit thread count
./openmp_monte_carlo_pi 5000000                          # num_tosses [threads]
./openmp_monte_carlo_pi 5000000 4                        # run with 4 threads
```

## Implementation notes

- **OpenMP histogram:** read all values on the host; each thread keeps a private bin array inside the parallel region, then merges into global bins (no locks in the inner loop). Values outside `[min_val, max_val]` are ignored; `x == max` goes to the last bin.
- **OpenMP Monte Carlo π:** reduction on the hit counter; each thread uses a local xorshift64* RNG so random streams differ; π = `4 * hits / tosses` using `long long` counts.

## Sample output

```text
./openmp_histogram 5 0.0 10.0 sample_data.txt
Histogram (bin edges [0, 10], 5 bins, 8 threads):
[0, 2) : 3
[2, 4) : 4
[4, 6) : 4
[6, 8) : 4
[8, 10)] : 5

./openmp_monte_carlo_pi 5000000
total_tosses=5000000  total_in_circle=3926648  pi_estimate=3.14131840  threads=8
```

## Clean

```bash
make clean
```
