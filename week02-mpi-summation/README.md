# Parallel Summation (Static & Dynamic)

This project has two MPI programs that compute the sum of N double values:

- `sum_static.c` - Static block decomposition using MPI_Scatterv and tree reduction
- `sum_dynamic.c` - Dynamic coordinator-worker with adaptive chunk sizing

## Note on Makefile

This Makefile uses `mpicc` to build the MPI programs `sum_static` and `sum_dynamic`.

## Requirements

- WSL2 (Ubuntu)
- Open MPI installed: `sudo apt install -y build-essential openmpi-bin libopenmpi-dev make`

## Build

```bash
make all
```

Or compile manually:
```bash
mpicc -O2 -std=c11 sum_static.c -o sum_static -lm
mpicc -O2 -std=c11 sum_dynamic.c -o sum_dynamic -lm
```

## Run

```bash
# Example with 4 processes
mpirun -np 4 ./sum_static 10000000
mpirun -np 4 ./sum_dynamic 10000000

# Test with different process counts
mpirun -np 1 ./sum_static 10000000
mpirun -np 2 ./sum_static 10000000
mpirun -np 4 ./sum_static 10000000
mpirun -np 8 ./sum_static 10000000
# Same for sum_dynamic
```

## How It Works

**sum_static.c:**
- Divides work upfront using MPI_Scatterv - each process gets a fixed chunk
- Processes work independently, then combine results using tree reduction
- Lower communication overhead, but can have load imbalance

**sum_dynamic.c:**
- Broadcasts full array to all processes using MPI_Bcast
- Rank 0 is coordinator, others are workers
- Workers request chunks dynamically, coordinator assigns based on performance
- More communication overhead, but better load balancing

## Output

Both programs print:
- N: number of elements
- p: number of processes
- total_sum: final sum
- checksum: for verification (should match between both programs)
- Runtime: elapsed time
