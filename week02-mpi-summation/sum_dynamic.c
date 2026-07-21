/*
 Dynamic Coordinator-Worker for Parallel Summation
 Author: Jose I Montero
 
 This implements the dynamic approach from Week 1 pseudocode.
 Uses SPMD (Single Program, Multiple Data) - learned from Pacheco & Malensek.
 Rank 0 is the coordinator, other ranks are workers.
 I'm using the Perry & Miller C book to understand the syntax.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

static uint32_t rand_state = 987654321u;

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

/* Check if busy minute */
static int is_busy_minute(int global_index) {
    if ((global_index % 101) == 0) {
        return 1;
    } else {
        return 0;
    }
}

static double verify_value(double x) {
    double acc = x;
    int i;
    for (i = 0; i < 32; i++) {
        acc = acc + sin(acc + i * 1e-3);
    }
    /* This should return x but adds computation time - the math cancels out */
    return (acc - (double)32 * sin(0)) * 0.0 + x;
}

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

/* Chapter 9 - enum */
enum { TAG_REQ = 1, TAG_WORK = 2, TAG_RESULT = 3, TAG_STOP = 4 };

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
    
    /* Chapter 10 - malloc, MPI_Bcast from slides and Pacheco & Malensek Ch 3
     all ranks need full array for dynamic work assignment */
    int num_bytes = sizeof(double) * (int)N;
    double *data = (double*)malloc(num_bytes);
    if (!data) {
        fprintf(stderr, "rank %d: alloc failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    if (rank == 0) {
        generate_data(data, (int)N);
    }
    MPI_Bcast(data, (int)N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    double total_sum = 0.0;
    double t_start = MPI_Wtime();
    
    if (rank == 0) {
        /* Special case: if only one process, coordinator does all work itself */
        if (size == 1) {
            double t0 = MPI_Wtime();
            long long i;
            for (i = 0; i < N; i++) {
                double v = data[i];
                if (is_busy_minute((int)i)) {
                    v = verify_value(v);
                }
                total_sum += v;
            }
            double t1 = MPI_Wtime();
            double t_end = MPI_Wtime();
            int cs = compute_checksum(data, (int)N);
            printf("sum_dynamic: N=%lld p=%d total_sum=%.6f checksum=%d runtime=%.6f seconds\n",
                   N, size, total_sum, cs, t_end - t_start);
            free(data);
            MPI_Finalize();
            return 0;
        }
        
        /* Chapter 5 - while loops, MPI_Probe/Recv/Send from slides and Pacheco & Malensek Ch 3
         Dynamic load balancing: coordinator adapts chunk size based on worker performance.
         Using MPI_Probe to check message type before receiving - learned this prevents
         MPI_ERR_TRUNCATE errors when message sizes differ. */
        long long next_index = 0;
        int workers_done = 0;
        double avg_time = 0.012;  /* starting guess, will adapt */
        const long long base_chunk = 1000;
        const long long min_chunk = 50;  /* too small = overhead */
        const long long max_chunk = 100000;  /* too big = imbalance */
        
        MPI_Status status;
        
        while (workers_done < (size - 1)) {
            /* Probe first to determine message tag and size before receiving */
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int src = status.MPI_SOURCE;
            int tag = status.MPI_TAG;
            
            if (tag == TAG_REQ) {
                MPI_Recv(NULL, 0, MPI_CHAR, src, TAG_REQ, MPI_COMM_WORLD, 
                        MPI_STATUS_IGNORE);
                
                if (next_index >= N) {
                    MPI_Send(NULL, 0, MPI_CHAR, src, TAG_STOP, MPI_COMM_WORLD);
                    workers_done++;
                } else {
                    /* Adaptive chunk sizing: if workers are fast, give bigger chunks.
                     If slow, give smaller chunks to improve load balance. */
                    double target = 0.012;
                    long long cs = (long long)(base_chunk * (target / avg_time));
                    
                    /* Clamp to reasonable bounds to avoid extreme sizes */
                    if (cs < min_chunk) cs = min_chunk;
                    if (cs > max_chunk) cs = max_chunk;
                    
                    long long lo = next_index;
                    long long hi = next_index + cs - 1;
                    if (hi >= N) hi = N - 1;  /* clamp to array bounds */
                    
                    /* send [lo, hi] inclusive range */
                    long long workbuf[2];
                    workbuf[0] = lo;
                    workbuf[1] = hi;
                    MPI_Send(workbuf, 2, MPI_LONG_LONG, src, TAG_WORK, 
                            MPI_COMM_WORLD);
                    next_index = hi + 1;
                }
            } 
            else if (tag == TAG_RESULT) {
                double msg_buf[2];
                MPI_Recv(msg_buf, 2, MPI_DOUBLE, src, TAG_RESULT, 
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                double part_sum = msg_buf[0];
                double elapsed = msg_buf[1];
                
                total_sum += part_sum;
                /* Exponential moving average: recent times weighted more heavily */
                avg_time = 0.3 * elapsed + 0.7 * avg_time;
            } 
            else if (tag == TAG_STOP) {
                MPI_Recv(NULL, 0, MPI_CHAR, src, TAG_STOP, MPI_COMM_WORLD, 
                        MPI_STATUS_IGNORE);
            }
        }
        double t_end = MPI_Wtime();
        int cs = compute_checksum(data, (int)N);
        printf("sum_dynamic: N=%lld p=%d total_sum=%.6f checksum=%d runtime=%.6f seconds\n",
               N, size, total_sum, cs, t_end - t_start);
        
    } else {
        MPI_Status status;
        
        while (1) {
            /* request work from coordinator */
            MPI_Send(NULL, 0, MPI_CHAR, 0, TAG_REQ, MPI_COMM_WORLD);
            
            /* wait for response - could be work or stop signal */
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int tag = status.MPI_TAG;
            
            if (tag == TAG_STOP) {
                MPI_Recv(NULL, 0, MPI_CHAR, 0, TAG_STOP, MPI_COMM_WORLD, 
                        MPI_STATUS_IGNORE);
                break;
            } 
            else if (tag == TAG_WORK) {
                long long workbuf[2];
                MPI_Recv(workbuf, 2, MPI_LONG_LONG, 0, TAG_WORK, 
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                long long lo = workbuf[0];
                long long hi = workbuf[1];
                
                double t0 = MPI_Wtime();
                double my_sum = 0.0;
                
                long long i;
                for (i = lo; i <= hi; i++) {
                    double v = data[i];
                    if (is_busy_minute((int)i)) {
                        v = verify_value(v);
                    }
                    my_sum += v;
                }
                
                double t1 = MPI_Wtime();
                double elapsed = t1 - t0;
                
                /* send back sum + time so coordinator can adapt chunk size */
                double outbuf[2];
                outbuf[0] = my_sum;
                outbuf[1] = elapsed;
                MPI_Send(outbuf, 2, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
            }
        }
    }
    
    /* Chapter 10 - free */
    free(data);
    
    MPI_Finalize();
    return 0;
}
