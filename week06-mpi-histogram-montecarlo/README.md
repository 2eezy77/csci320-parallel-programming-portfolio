# Week 6 Programming Assignments (MPI)

Two small MPI programs for Chapter 3 assignments:

- `histogram`: reads a list of numbers on rank 0, distributes them, and prints the histogram on rank 0.
- `monte_carlo_pi`: estimates π with a Monte Carlo dart toss and reduces the results to rank 0.

## Build

```bash
cd Week\ 6/Assignments/Montero_MPI_Histogram_and_MonteCarloPi
make
```

## histogram

Implements the Section 2.7.1 histogram example. Rank 0 reads all values, scatters work, every rank counts locally, and rank 0 prints the combined counts.

Usage:

```bash
mpirun -np 4 ./histogram <num_bins> <min_val> <max_val> <data_file>
```

Example:

```bash
mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt
```

Notes:

- Inputs outside `[min_val, max_val]` are ignored.
- Values equal to `max_val` are placed in the last bin.

## monte_carlo_pi

Implements the Monte Carlo dartboard estimate of π from the assignment. Rank 0 reads the total tosses, broadcasts to everyone, each rank tosses independently, and rank 0 prints the global π estimate.

Usage:

```bash
mpirun -np 4 ./monte_carlo_pi <num_tosses> [seed]
```

Examples:

```bash
mpirun -np 4 ./monte_carlo_pi 1000000
mpirun -np 8 ./monte_carlo_pi 5000000 12345
```

### Output

Rank 0 prints the total tosses, total points inside the unit circle, and the π estimate:

```
total_tosses=1000000  total_in_circle=785398  pi_estimate=3.141592
```

## Cleaning up

```bash
make clean
```
