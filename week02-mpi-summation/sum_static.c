/*
 Static Block Decomposition for Parallel Summation
 Author: Jose I Montero
 
 This implements the static approach from Week 1 pseudocode.
 Uses SPMD (Single Program, Multiple Data) - learned from Pacheco & Malensek.
 I'm learning C coming from Java, so I'm using the Perry & Miller book
 to understand pointers, malloc, and MPI functions.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/* Random number generator - different from Java's Random class */
static uint32_t rand_state = 987654321u;

/* Chapter 8 - functions */
static uint32_t random_next(void) {
    rand_state = 1664525u * rand_state + 1013904223u;
    return rand_state;
}

/* Chapter 10 - pointers */
static void generate_data(double *arr, int N) {
    int i;
    for (i = 0; i < N; i++) {
        uint32_t r = random_next();
        int temp = (int)(r % 10001);
        arr[i] = ((double)(temp - 5000)) / 1000.0;
    }
}

/* Check if busy minute - every 101st element */
static int is_busy_minute(int global_index) {
    if ((global_index % 101) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* Extra computation for busy minutes */
static double verify_value(double x) {
    double acc = x;
    int i;
    for (i = 0; i < 32; i++) {
        acc = acc + sin(acc + i * 1e-3);
    }
    /* This should return x but adds computation time - the math cancels out */
    return (acc - (double)32 * sin(0)) * 0.0 + x;
}

/* Compute checksum */
static int compute_checksum(const double *arr, int N) {
    long long s = 0;
    int i;
    for (i = 0; i < N; i++) {
        double abs_val = fabs(arr[i]);
        long long v = (long long)(abs_val * 1000.0);
        s = s + v;
    }
    return (int)(s % 7);
}

/* Chapter 2 - main, argc, argv */
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s N\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    long long N = atoll(argv[1]);
    if (N <= 0) {
        if (rank == 0) {
            fprintf(stderr, "N must be positive\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    /* Chapter 10 - malloc - only rank 0 generates data, others get it via scatter */
    double *data = NULL;
    if (rank == 0) {
        int num_bytes = sizeof(double) * (int)N;
        data = (double*)malloc(num_bytes);
        if (!data) {
            fprintf(stderr, "rank0: alloc failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        generate_data(data, N);
    }
    
    /* Chapter 9 - arrays - Static decomposition: divide work evenly, with leftover elements
     going to first few processes. This ensures load balance upfront. */
    int *counts = (int*)malloc(sizeof(int) * size);
    int *displs = (int*)malloc(sizeof(int) * size);
    
    long long base = N / size;
    int leftover = (int)(N % size);
    
    int r;
    for (r = 0; r < size; r++) {
        if (r < leftover) {
            counts[r] = (int)(base + 1);
        } else {
            counts[r] = (int)base;
        }
    }
    
    /* Calculate displacements for MPI_Scatterv */
    displs[0] = 0;
    for (r = 1; r < size; r++) {
        displs[r] = displs[r-1] + counts[r-1];
    }
    
    int my_count = counts[rank];
    int buf_size = sizeof(double) * my_count;
    double *my_buf = (double*)malloc(buf_size);
    if (!my_buf) {
        fprintf(stderr, "rank %d: alloc failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    /* MPI_Scatterv from slides and Pacheco & Malensek Ch 3 */
    MPI_Scatterv(data, counts, displs, MPI_DOUBLE, my_buf, my_count, 
                 MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    long long my_start = displs[rank];
    double my_sum = 0.0;
    double t0 = MPI_Wtime();
    
    int i;
    for (i = 0; i < my_count; i++) {
        double v = my_buf[i];
        int gidx = (int)(my_start + i);  /* global index for busy minute check */
        if (is_busy_minute(gidx)) {
            v = verify_value(v);
        }
        my_sum += v;
    }
    
    double t1 = MPI_Wtime();
    double local_elapsed = t1 - t0;
    
    /* Chapter 5 - while loops, MPI_Send/Recv from slides and Pacheco & Malensek Ch 3
     Tree-based reduction: log(p) steps instead of p-1 sequential sends.
     Each step doubles the distance, so we get O(log p) complexity. */
    int p = size;
    int distance = 1;
    while (distance < p) {
        if ((rank % (2*distance)) == 0) {
            int partner = rank + distance;
            if (partner < p) {
                double partner_sum;
                MPI_Recv(&partner_sum, 1, MPI_DOUBLE, partner, 0, 
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_sum += partner_sum;
            }
        } 
        else if ((rank % (2*distance)) == distance) {
            /* send to lower rank and exit - done after this */
            int partner = rank - distance;
            MPI_Send(&my_sum, 1, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD);
            break;
        }
        distance *= 2;
    }
    
    if (rank == 0) {
        int cs = compute_checksum(data, (int)N);
        printf("sum_static: N=%lld p=%d total_sum=%.6f checksum=%d local_elapsed(of rank0)=%.6f\n",
               N, size, my_sum, cs, local_elapsed);
    }
    
    /* Chapter 10 - free */
    free(counts);
    free(displs);
    free(my_buf);
    if (data) {
        free(data);
    }
    
    MPI_Finalize();
    return 0;
}
