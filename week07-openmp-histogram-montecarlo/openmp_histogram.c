// Jose I. Montero - Week 7 OpenMP Histogram
// CSCI 320: Parallel & Distributive Programming
// My OpenMP take on the book's histogram exercise (Chapter 2/5).
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static double *read_values(const char *path, int *out_count) {
    FILE *f = fopen(path, "r");
    if (!f) die("Could not open data file.");

    int cap = 1024;
    int n = 0;
    double *vals = malloc((size_t)cap * sizeof(double));
    if (!vals) die("Allocation failed.");

    double tmp;
    while (fscanf(f, "%lf", &tmp) == 1) {
        if (n == cap) {
            cap *= 2;
            double *new_vals = realloc(vals, (size_t)cap * sizeof(double));
            if (!new_vals) die("Realloc failed.");
            vals = new_vals;
        }
        vals[n++] = tmp;
    }
    fclose(f);
    *out_count = n;
    return vals;
}

int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 6) {
        fprintf(stderr, "Usage: %s <num_bins> <min_val> <max_val> <data_file> [num_threads]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_bins = atoi(argv[1]);
    double min_val = atof(argv[2]);
    double max_val = atof(argv[3]);
    const char *file_path = argv[4];
    int num_threads = (argc == 6) ? atoi(argv[5]) : 0;

    if (num_bins <= 0 || max_val <= min_val || (argc == 6 && num_threads <= 0)) {
        die("Bad args (bins>0, max>min, threads>0 if provided).");
    }
    if (num_threads > 0) omp_set_num_threads(num_threads);

    double bin_width = (max_val - min_val) / num_bins;

    int total_vals = 0;
    double *values = read_values(file_path, &total_vals);
    if (total_vals == 0) {
        free(values);
        die("No data read from file.");
    }

    long long *global_bins = calloc((size_t)num_bins, sizeof(long long));
    if (!global_bins) die("Allocation failed for bins.");

    // Parallel region: each thread keeps its own bins, then we combine.
    #pragma omp parallel
    {
        long long *local_bins = calloc((size_t)num_bins, sizeof(long long));
        if (!local_bins) die("Allocation failed for local bins.");

        // Split the work over the input values.
        #pragma omp for nowait
        for (int i = 0; i < total_vals; i++) {
            double x = values[i];
            if (x < min_val || x > max_val) continue; // drop out-of-range values
            int bin = (int)((x - min_val) / bin_width);
            if (bin == num_bins) bin = num_bins - 1; // catch x == max
            local_bins[bin]++;
        }

        // Simple critical section to merge thread-local bins.
        #pragma omp critical
        {
            for (int b = 0; b < num_bins; b++) {
                global_bins[b] += local_bins[b];
            }
        }
        free(local_bins);
    }

    printf("Histogram (bin edges [%g, %g], %d bins, %d threads):\n",
           min_val, max_val, num_bins, omp_get_max_threads());
    for (int b = 0; b < num_bins; b++) {
        double left = min_val + b * bin_width;
        double right = (b == num_bins - 1) ? max_val : left + bin_width;
        printf("[%g, %g)%s : %lld\n",
               left,
               right,
               (b == num_bins - 1) ? "]" : "",
               global_bins[b]);
    }

    free(global_bins);
    free(values);
    return EXIT_SUCCESS;
}
