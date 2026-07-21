// Jose I. Montero - Week 6 PThreads Monte Carlo Pi (CSCI 320: Parallel & Distributive Programming)
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    long long start;
    long long end;
    uint64_t seed;
    long long hits;
} thread_arg_t;

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

// xorshift64* RNG; fast and good enough for Monte Carlo
static inline uint64_t xorshift64star(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

// Get a uniform double in [0,1).
static inline double rand_unit(uint64_t *state) {
    return (double)(xorshift64star(state) >> 11) * (1.0 / 9007199254740992.0); // [0,1)
}

// Thread entry: run assigned tosses with a private RNG and count hits.
static void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    uint64_t seed = a->seed;
    long long local_hits = 0;
    // Independent toss loop; no synchronization inside.
    for (long long toss = a->start; toss < a->end; toss++) {
        double x = 2.0 * rand_unit(&seed) - 1.0; // [-1,1]
        double y = 2.0 * rand_unit(&seed) - 1.0; // [-1,1]
        double dist2 = x * x + y * y;
        if (dist2 <= 1.0) local_hits++;
    }
    a->hits = local_hits;
    return NULL;
}

// Parse args, partition tosses, launch threads, and report pi estimate.
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <num_tosses> [num_threads]\n", argv[0]);
        return EXIT_FAILURE;
    }

    long long total_tosses = atoll(argv[1]);
    if (total_tosses <= 0) die("num_tosses must be positive.");

    int num_threads = (argc == 3) ? atoi(argv[2]) : 4;
    if (num_threads <= 0) die("num_threads must be positive.");

    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    thread_arg_t *args = malloc((size_t)num_threads * sizeof(thread_arg_t));
    if (!threads || !args) die("Allocation failed for threads/args.");

    // Static partition of tosses; first 'rem' threads get one extra toss.
    long long base = total_tosses / num_threads;
    long long rem = total_tosses % num_threads;
    long long cursor = 0;
    uint64_t base_seed = (uint64_t)time(NULL);

    for (int t = 0; t < num_threads; t++) {
        long long chunk = base + (t < rem ? 1 : 0);
        args[t].start = cursor;
        args[t].end = cursor + chunk;
        args[t].seed = base_seed ^ (UINT64_C(0x9e3779b97f4a7c15) + (uint64_t)t);
        args[t].hits = 0;
        cursor += chunk;

        if (pthread_create(&threads[t], NULL, worker, &args[t]) != 0) {
            die("pthread_create failed.");
        }
    }

    long long total_hits = 0;
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
        total_hits += args[t].hits;
    }

    double pi_est = 4.0 * (double)total_hits / (double)total_tosses;
    printf("total_tosses=%lld  total_in_circle=%lld  pi_estimate=%.8f  threads=%d\n",
           total_tosses, total_hits, pi_est, num_threads);

    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
