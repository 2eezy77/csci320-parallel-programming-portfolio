// Jose I. Montero - Week 7 OpenMP Monte Carlo Pi
// CSCI 320: Parallel & Distributive Programming
// My OpenMP version of the Monte Carlo pi tosses (Chapter 5).
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

// Quick xorshift64* so each thread gets its own random stream
static inline uint64_t xorshift64star(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

static inline double rand_unit(uint64_t *state) {
    return (double)(xorshift64star(state) >> 11) * (1.0 / 9007199254740992.0); // [0,1)
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <num_tosses> [num_threads]\n", argv[0]);
        return EXIT_FAILURE;
    }

    long long total_tosses = atoll(argv[1]);
    if (total_tosses <= 0) die("num_tosses must be positive.");

    if (argc == 3) {
        int threads = atoi(argv[2]);
        if (threads <= 0) die("num_threads must be positive.");
        omp_set_num_threads(threads);
    }

    long long hits_in_circle = 0;
    uint64_t base_seed = (uint64_t)time(NULL);

    // Parallel region with a reduction on the hit counter.
    #pragma omp parallel reduction(+:hits_in_circle)
    {
        int tid = omp_get_thread_num();
        uint64_t seed = base_seed ^ (UINT64_C(0x9e3779b97f4a7c15) + (uint64_t)tid);
        // Let OpenMP hand out toss iterations; each thread uses its own seed.
        #pragma omp for
        for (long long toss = 0; toss < total_tosses; toss++) {
            double x = 2.0 * rand_unit(&seed) - 1.0; // [-1,1]
            double y = 2.0 * rand_unit(&seed) - 1.0; // [-1,1]
            double dist2 = x * x + y * y;
            if (dist2 <= 1.0) hits_in_circle++;
        }
    }

    double pi_est = 4.0 * (double)hits_in_circle / (double)total_tosses;
    printf("total_tosses=%lld  total_in_circle=%lld  pi_estimate=%.8f  threads=%d\n",
           total_tosses, hits_in_circle, pi_est, omp_get_max_threads());
    return EXIT_SUCCESS;
}
