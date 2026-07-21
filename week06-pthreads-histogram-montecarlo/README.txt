Week 6: PThreads Histogram and Monte Carlo Pi
Jose I. Montero - CSCI 320 Parallel & Distributive Programming

What's here
- pthreads_histogram.c : Pthreads histogram (Chapter 2/4 assignment)
- pthreads_monte_carlo_pi.c : Pthreads Monte Carlo pi (Chapter 4 assignment)
- Makefile : builds the two programs
- sample_data.txt : tiny test input for the histogram

Build
  make all # Compiles both programs
  
clean
  make clean # Compile both

Run examples
  ./pthreads_histogram 5 0.0 10.0 sample_data.txt 4        # explicit thread count
  ./pthreads_monte_carlo_pi 5000000 4                      # run with 4 threads

Notes on implementation
- Histogram: main thread reads all values, slices work by index range, each
  thread owns a private bin array, then main reduces after join (no locks needed).
- Monte Carlo pi: static work partition by toss count; each thread gets its own
  xorshift64* RNG stream; pi = 4 * hits / tosses using long long counts.

Outputs
 - ../Montero_PThreads_Histogram_and_MonteCarloPi$ ./pthreads_histogram 5 0.0 10.0 sample_data.txt 4
Histogram (bin edges [0, 10], 5 bins, 4 threads):
[0, 2) : 3
[2, 4) : 4
[4, 6) : 4
[6, 8) : 4
[8, 10)] : 5

 - ../Montero_PThreads_Histogram_and_MonteCarloPi$ ./pthreads_monte_carlo_pi 5000000 4
total_tosses=5000000  total_in_circle=3927176  pi_estimate=3.14174080  threads=4