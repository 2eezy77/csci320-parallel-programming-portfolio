# Week 6: Histogram and Monte Carlo π (MPI)

Jose I. Montero — CSCI 320 Parallel & Distributed Programming

## Contents

- `histogram.c` — MPI scatter/reduce histogram
- `monte_carlo_pi.c` — MPI Monte Carlo π estimator
- `Makefile` — builds with `mpicc`
- `sample_data.txt` — tiny test input for the histogram

## Build

```bash
make
```

## Run examples

```bash
mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt
mpirun -np 4 ./monte_carlo_pi 1000000
mpirun -np 8 ./monte_carlo_pi 5000000 12345   # with seed
```

## Notes (histogram)

- Rank 0 reads all doubles from the data file, then scatters them across ranks.
- Each rank ignores values outside `[min_val, max_val]`; values equal to `max_val` go in the last bin.
- Rank 0 reduces and prints the global bin counts with bin edges.

## Notes (Monte Carlo π)

- Rank 0 broadcasts total tosses (and optional seed); each rank tosses locally.
- Uses `long long` for toss and hit counts; `MPI_Reduce` sums hits to rank 0.
- π estimate = `4 * (global hits) / (total tosses)`.
- Uses `srand` / `rand` for portability.

## Sample output

```text
mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt
Histogram (bin edges [0, 10], 5 bins):
[0, 2) : 3
[2, 4) : 4
[4, 6) : 4
[6, 8) : 4
[8, 10)] : 5

mpirun -np 4 ./monte_carlo_pi 5000000 12345
total_tosses=5000000  total_in_circle=3926648  pi_estimate=3.14131840
```

## Clean

```bash
make clean
```
