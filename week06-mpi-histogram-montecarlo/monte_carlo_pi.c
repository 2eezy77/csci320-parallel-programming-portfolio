// Jose I. Montero - Week 6 Monte Carlo Pi (MPI)
// CSCI 320: Parallel & Distributive Programming
// Monte Carlo dartboard estimate of pi with a simple broadcast/reduce.
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (argc < 2 || argc > 3) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <num_tosses> [seed]\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    long long total_tosses = 0;
    if (rank == 0) {
        total_tosses = atoll(argv[1]);
        if (total_tosses <= 0) {
            fprintf(stderr, "num_tosses must be positive.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast toss count
    MPI_Bcast(&total_tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Seed setup: allow optional user seed (use rand for portability)
    long long base_seed = (argc == 3 && rank == 0) ? atoll(argv[2])
                                                   : (long long)time(NULL);
    MPI_Bcast(&base_seed, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    unsigned int seed = (unsigned int)(base_seed + 9973 * rank);
    srand(seed);

    // Divide work
    long long local_tosses = total_tosses / comm_sz;
    long long remainder = total_tosses % comm_sz;
    if (rank < remainder) local_tosses++;

    long long local_in_circle = 0;
    for (long long toss = 0; toss < local_tosses; toss++) {
        double x = 2.0 * (double)rand() / (double)RAND_MAX - 1.0; // [-1,1]
        double y = 2.0 * (double)rand() / (double)RAND_MAX - 1.0; // [-1,1]
        double distance_squared = x * x + y * y;
        if (distance_squared <= 1.0) local_in_circle++;
    }

    long long global_in_circle = 0;
    MPI_Reduce(&local_in_circle, &global_in_circle, 1, MPI_LONG_LONG, MPI_SUM,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi_estimate = 4.0 * (double)global_in_circle /
                             (double)total_tosses;
        printf("total_tosses=%lld  total_in_circle=%lld  pi_estimate=%.8f\n",
               total_tosses, global_in_circle, pi_estimate);
    }

    MPI_Finalize();
    return 0;
}
