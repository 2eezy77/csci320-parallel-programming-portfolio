# Parallel Programming Portfolio (CSCI 320)

Coursework from **CSCI 320 Parallel and Distributed Programming** (Regent University).

C implementations of the same core problems across three parallel models: **MPI**, **Pthreads**, and **OpenMP**.

**Author:** Jose I. Montero

## Projects

| Folder | Model | What it shows |
| ------ | ----- | ------------- |
| `week02-mpi-summation` | MPI | Static vs dynamic load balancing for parallel summation |
| `week06-mpi-histogram-montecarlo` | MPI | Histogram + Monte Carlo π with scatter/reduce patterns |
| `week06-pthreads-histogram-montecarlo` | Pthreads | Shared-memory threading and per-thread partitioning |
| `week07-openmp-histogram-montecarlo` | OpenMP | Parallel loops with reductions |

## Requirements

- GCC (or equivalent) with OpenMP support
- Open MPI (`mpicc`, `mpirun`) for MPI projects
- `make`

On Ubuntu / WSL:

```bash
sudo apt install -y build-essential openmpi-bin libopenmpi-dev
```

## How to reproduce results

Measured on the course machine for week 2 parallel summation with **N = 10,000,000** and seed `987654321` (same data for static and dynamic). Full table: [`week02-mpi-summation/PERFORMANCE_REPORT.md`](week02-mpi-summation/PERFORMANCE_REPORT.md).

```bash
cd week02-mpi-summation
make all

# Static (block decomposition + tree reduction)
mpirun -np 1 ./sum_static 10000000
mpirun -np 2 ./sum_static 10000000
mpirun -np 4 ./sum_static 10000000
mpirun -np 8 ./sum_static 10000000

# Dynamic (coordinator-worker, adaptive chunks)
mpirun -np 1 ./sum_dynamic 10000000
mpirun -np 2 ./sum_dynamic 10000000
mpirun -np 4 ./sum_dynamic 10000000
mpirun -np 8 ./sum_dynamic 10000000
```

Sample runtimes (seconds) and speedup vs 1 process:

| Processes | Static (s) | Dynamic (s) | Static speedup | Dynamic speedup |
| --------- | ---------- | ----------- | -------------- | --------------- |
| 1         | 0.0553     | 0.0453      | 1.00x          | 1.00x           |
| 2         | 0.0276     | 0.0415      | 2.01x          | 1.09x           |
| 4         | 0.0139     | 0.0842      | 3.97x          | 0.54x           |
| 8         | 0.0071     | 0.0062      | 7.75x          | 7.33x           |

Both variants matched checksum `1` and total sum `7464.454000`.

Histogram / Monte Carlo builds (same problem, three models):

```bash
cd week06-mpi-histogram-montecarlo && make && mpirun -np 4 ./monte_carlo_pi 5000000 12345
cd ../week06-pthreads-histogram-montecarlo && make && ./pthreads_monte_carlo_pi 5000000 4
cd ../week07-openmp-histogram-montecarlo && make && ./openmp_monte_carlo_pi 5000000 4
```

Each project folder has its own `Makefile` and README with program-specific flags.

## Model comparison (same problem)

Histogram and Monte Carlo π are implemented three ways. Course reports timed week 2 summation in detail; cross-model wall-clock timings for histogram / Monte Carlo were not recorded in those reports. Use the same inputs below for a fair local comparison (`N = 5,000,000` tosses; histogram uses `sample_data.txt`).

| Problem | MPI | Pthreads | OpenMP | Notes |
| ------- | --- | -------- | ------ | ----- |
| Monte Carlo π (`N=5e6`) | `mpirun -np 4 ./monte_carlo_pi 5000000 12345` | `./pthreads_monte_carlo_pi 5000000 4` | `./openmp_monte_carlo_pi 5000000 4` | Sample π estimates on the course machine: MPI ≈ 3.14131840, Pthreads ≈ 3.14174080, OpenMP ≈ 3.14131840 (RNG / partition differ by model) |
| Histogram (tiny file) | `mpirun -np 4 ./histogram 5 0.0 10.0 sample_data.txt` | `./pthreads_histogram 5 0.0 10.0 sample_data.txt 4` | `./openmp_histogram 5 0.0 10.0 sample_data.txt 4` | All three print the same bin counts on `sample_data.txt` |
| Parallel sum (`N=1e7`) | Static vs dynamic (table above) | — | — | Quantitative MPI-only; see performance report |

Qualitative trade-offs for this workload:

| Model | Strength | Cost |
| ----- | -------- | ---- |
| MPI | Scales across processes / nodes; clear scatter/reduce | Process startup and message overhead |
| Pthreads | Shared memory, fine control of partitioning | Explicit thread setup / join |
| OpenMP | Compact parallel loops and reductions | Less explicit control than hand-written threads |

## Skills demonstrated

- Process vs thread parallelism
- Load balancing (static block vs dynamic work distribution)
- Collective communication and reductions
- Shared-memory synchronization patterns
- Performance comparison across process/thread counts

## License

MIT License. See [`LICENSE`](LICENSE).

## Note

This repository contains only the programming assignments needed to evaluate the work. Course notes, slides, and textbooks are not included.
