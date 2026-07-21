Week 6/7: Histogram and Monte Carlo Pi (MPI + Pthreads)
Jose I. Montero - CSCI 320 Parallel & Distributive Programming

Contents
- histogram.c : MPI scatter/reduce histogram (earlier Chapter 3 assignment)
- monte_carlo_pi.c : MPI Monte Carlo pi estimator (earlier Chapter 3 assignment)
- pthread_histogram.c : Pthreads histogram (Chapter 4.1 programming assignment)
- pthread_monte_carlo_pi.c : Pthreads Monte Carlo pi estimator (Chapter 4.2 programming assignment)
- Makefile : builds MPI and Pthreads binaries (uses mpicc for MPI, gcc -pthread for Pthreads)
- sample_data.txt : tiny test input for the histogram

Build
  make

Run examples
  mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt
  mpirun -np 4 ./monte_carlo_pi 1000000
  mpirun -np 8 ./monte_carlo_pi 5000000 12345   # with seed
  ./pthread_histogram 5 0.0 10.0 sample_data.txt 4
  ./pthread_monte_carlo_pi 5000000 4 12345      # <tosses> <threads> [seed]

Notes (histogram)
- Rank 0 reads all doubles from the data file, scatters them across ranks.
- Each rank ignores values outside [min_val, max_val]; values == max_val go in the last bin.
- Rank 0 reduces and prints the global bin counts with bin edges.

Notes (Monte Carlo pi)
- Rank 0 broadcasts total tosses (and optional seed); each rank tosses locally.
- Uses long long for toss and hit counts; MPI_Reduce sums hits to rank 0.
- pi estimate = 4 * (global hits) / (total tosses).
- Switched to srand/rand for portability.

Notes (Pthreads histogram)
- Main thread reads all values, partitions them across threads, each thread builds a private bin array.
- Uses long long bin counts; ignores values outside [min_val, max_val]; values == max_val fall into the last bin.

Notes (Pthreads Monte Carlo pi)
- Main thread divides tosses across threads; each thread uses a thread-local xorshift64* RNG for reproducibility.
- Uses long long for toss and hit counts; pi = 4 * (global hits) / (total tosses).

Sample output (from my machine)
  mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt
  Histogram (bin edges [0, 10], 5 bins):
  [0, 2) : 3
  [2, 4) : 4
  [4, 6) : 4
  [6, 8) : 4
  [8, 10)] : 5

  mpirun -np 4 ./monte_carlo_pi 5000000 12345
  total_tosses=5000000  total_in_circle=3926648  pi_estimate=3.14131840
