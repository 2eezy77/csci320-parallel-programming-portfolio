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

## Build and run (example)

```bash
cd week02-mpi-summation
make all
mpirun -np 4 ./sum_static 10000000
mpirun -np 4 ./sum_dynamic 10000000
```

Each project folder has its own `Makefile` and short README with program-specific flags.

## Skills demonstrated

- Process vs thread parallelism
- Load balancing (static block vs dynamic work distribution)
- Collective communication and reductions
- Shared-memory synchronization patterns
- Performance comparison across process/thread counts

## Note

This repository contains only the programming assignments needed to evaluate the work. Course notes, slides, and textbooks are not included.
