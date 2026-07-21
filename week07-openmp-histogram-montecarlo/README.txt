Week 7: Histogram and Monte Carlo Pi (OpenMP)
Jose I. Montero - CSCI 320 Parallel & Distributive Programming

What’s here
- openmp_histogram.c : OpenMP histogram (Chapter 2/5 assignment)
- openmp_monte_carlo_pi.c : OpenMP Monte Carlo pi (Chapter 5 assignment)
- Makefile : builds the two OpenMP programs (`gcc -fopenmp`)
- sample_data.txt : tiny test input for the histogram

Build
  make

How I ran them
  ./openmp_histogram 5 0.0 10.0 sample_data.txt            # bins min max file [threads]
  ./openmp_histogram 5 0.0 10.0 sample_data.txt 4          # with explicit thread count
  ./openmp_monte_carlo_pi 5000000                          # num_tosses [threads]
  ./openmp_monte_carlo_pi 5000000 4                        # run with 4 threads

Notes on what I did
- OpenMP histogram: read all values on the host, each thread keeps a private bin array inside the parallel region, then I add them into global bins (no locks in the inner loop). Values outside [min_val, max_val] are ignored; x==max goes to the last bin.
- OpenMP Monte Carlo pi: reduction on the hit counter; each thread uses a local xorshift64* RNG so random streams differ; pi = 4 * hits / tosses using long long counts.

Sample output I saw
  ./openmp_histogram 5 0.0 10.0 sample_data.txt
  Histogram (bin edges [0, 10], 5 bins, 8 threads):
  [0, 2) : 3
  [2, 4) : 4
  [4, 6) : 4
  [6, 8) : 4
  [8, 10)] : 5

  ./openmp_monte_carlo_pi 5000000
  total_tosses=5000000  total_in_circle=3926648  pi_estimate=3.14131840  threads=8
