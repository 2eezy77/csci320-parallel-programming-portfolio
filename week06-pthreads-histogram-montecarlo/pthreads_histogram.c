// Jose I. Montero - Week 6 PThreads Histogram (CSCI 320: Parallel & Distributive Programming)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const double *vals;
    int start;
    int end;
    int num_bins;
    double min_val;
    double max_val;
    double bin_width;
    long long *local_bins;
} thread_arg_t;

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

// Print usage and exit failure from caller.
static void usage(void) {
    fprintf(stderr,
            "Usage: pthreads_histogram <num_bins> <min_val> <max_val> <data_file> [num_threads]\n");
}

// Thread entry: count local elements into a private bin array.
static void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    // Per-thread accumulation: no sharing, no locks.
    for (int i = a->start; i < a->end; i++) {
        double x = a->vals[i];
        if (x < a->min_val || x > a->max_val) continue; // ignore out-of-range
        int bin = (int)((x - a->min_val) / a->bin_width);
        if (bin == a->num_bins) bin = a->num_bins - 1;   // handle x == max_val
        a->local_bins[bin]++;
    }
    return NULL;
}

// Parse inputs, read data, launch threads, and print the combined histogram.
int main(int argc, char *argv[]) {
    if (argc < 5 || argc > 6) {
        usage();
        return EXIT_FAILURE;
    }

    int num_bins = atoi(argv[1]);
    double min_val = atof(argv[2]);
    double max_val = atof(argv[3]);
    const char *file_path = argv[4];
    int num_threads = (argc == 6) ? atoi(argv[5]) : 4;

    if (num_bins <= 0 || max_val <= min_val) die("Invalid bins/min/max arguments.");
    if (num_threads <= 0) die("num_threads must be positive.");

    double bin_width = (max_val - min_val) / num_bins;

    // Read all values into memory; single-threaded I/O avoids contention.
    FILE *f = fopen(file_path, "r");
    if (!f) die("Could not open data file.");

    int cap = 1024;
    int total_vals = 0;
    double *vals = malloc((size_t)cap * sizeof(double));
    if (!vals) die("Allocation failed.");

    while (fscanf(f, "%lf", &vals[total_vals]) == 1) {
        total_vals++;
        if (total_vals == cap) {
            cap *= 2;
            double *tmp = realloc(vals, (size_t)cap * sizeof(double));
            if (!tmp) die("Realloc failed.");
            vals = tmp;
        }
    }
    fclose(f);

    if (total_vals == 0) die("No data read from file.");

    // Thread setup.
    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    thread_arg_t *args = malloc((size_t)num_threads * sizeof(thread_arg_t));
    long long **local_bins_arr = malloc((size_t)num_threads * sizeof(long long *));
    if (!threads || !args || !local_bins_arr) die("Allocation failed for threads/args.");

    // Static partition: first 'rem' threads take one extra element.
    int base = total_vals / num_threads;
    int rem = total_vals % num_threads;
    int cursor = 0;
    for (int t = 0; t < num_threads; t++) {
        int chunk = base + (t < rem ? 1 : 0);
        int start = cursor;
        int end = cursor + chunk;
        cursor = end;

        long long *local_bins = calloc((size_t)num_bins, sizeof(long long));
        if (!local_bins) die("Allocation failed for local_bins.");
        local_bins_arr[t] = local_bins;

        args[t] = (thread_arg_t){
            .vals = vals,
            .start = start,
            .end = end,
            .num_bins = num_bins,
            .min_val = min_val,
            .max_val = max_val,
            .bin_width = bin_width,
            .local_bins = local_bins};

        if (pthread_create(&threads[t], NULL, worker, &args[t]) != 0) {
            die("pthread_create failed.");
        }
    }

    // Join and reduce.
    long long *global_bins = calloc((size_t)num_bins, sizeof(long long));
    if (!global_bins) die("Allocation failed for global_bins.");

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
        for (int b = 0; b < num_bins; b++) {
            global_bins[b] += local_bins_arr[t][b];
        }
        free(local_bins_arr[t]);
    }

    // Output.
    printf("Histogram (bin edges [%g, %g], %d bins, %d threads):\n",
           min_val, max_val, num_bins, num_threads);
    for (int b = 0; b < num_bins; b++) {
        double left = min_val + b * bin_width;
        double right = (b == num_bins - 1) ? max_val : left + bin_width;
        printf("[%g, %g)%s : %lld\n",
               left,
               right,
               (b == num_bins - 1) ? "]" : "",
               global_bins[b]);
    }

    free(vals);
    free(threads);
    free(args);
    free(local_bins_arr);
    free(global_bins);
    return EXIT_SUCCESS;
}
