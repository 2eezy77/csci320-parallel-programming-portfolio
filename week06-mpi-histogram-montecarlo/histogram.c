// Jose I. Montero - Week 6 MPI Histogram
// CSCI 320: Parallel & Distributive Programming
// Simple MPI scatter/reduce version of the textbook histogram example.
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
}

static void usage(int rank) {
    if (rank == 0) {
        fprintf(stderr,
                "Usage: histogram <num_bins> <min_val> <max_val> <data_file>\n");
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (argc != 5) {
        usage(rank);
        MPI_Finalize();
        return 1;
    }

    int num_bins = atoi(argv[1]);
    double min_val = atof(argv[2]);
    double max_val = atof(argv[3]);
    const char *file_path = argv[4];

    if (num_bins <= 0 || max_val <= min_val) {
        if (rank == 0) {
            fprintf(stderr, "Invalid bins/min/max arguments.\n");
        }
        MPI_Finalize();
        return 1;
    }

    double bin_width = (max_val - min_val) / num_bins;

    double *all_vals = NULL;
    int total_vals = 0;
    if (rank == 0) {
        FILE *f = fopen(file_path, "r");
        if (!f) die("Could not open data file.");

        // Rough read: count numbers first
        double tmp;
        int cap = 1024;
        all_vals = malloc((size_t)cap * sizeof(double));
        if (!all_vals) die("Allocation failed.");

        while (fscanf(f, "%lf", &tmp) == 1) {
            if (total_vals == cap) {
                cap *= 2;
                double *new_arr = realloc(all_vals, (size_t)cap * sizeof(double));
                if (!new_arr) die("Realloc failed.");
                all_vals = new_arr;
            }
            all_vals[total_vals++] = tmp;
        }
        fclose(f);
    }

    // Broadcast the number of values to everyone
    MPI_Bcast(&total_vals, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Compute scatter counts and displacements on root
    int *counts = malloc((size_t)comm_sz * sizeof(int));
    int *displs = malloc((size_t)comm_sz * sizeof(int));
    if (!counts || !displs) die("Allocation failed for counts/displs.");

    int base = total_vals / comm_sz;
    int rem = total_vals % comm_sz;
    for (int i = 0, disp = 0; i < comm_sz; i++) {
        counts[i] = base + (i < rem ? 1 : 0);
        displs[i] = disp;
        disp += counts[i];
    }

    // Allocate local buffer and scatter
    int local_n = counts[rank];
    double *local_vals = NULL;
    if (local_n > 0) {
        local_vals = malloc((size_t)local_n * sizeof(double));
        if (!local_vals) die("Allocation failed for local_vals.");
    }

    MPI_Scatterv(all_vals, counts, displs, MPI_DOUBLE,
                 local_vals, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Local histogram (each rank counts its chunk)
    long long *local_bins = calloc((size_t)num_bins, sizeof(long long));
    if (!local_bins) die("Allocation failed for local_bins.");

    for (int i = 0; i < local_n; i++) {
        double x = local_vals[i];
        if (x < min_val || x > max_val) continue; // ignore out-of-range
        int bin = (int)((x - min_val) / bin_width);
        if (bin == num_bins) bin = num_bins - 1;   // edge case x == max_val
        local_bins[bin]++;
    }

    long long *global_bins = NULL;
    if (rank == 0) {
        global_bins = calloc((size_t)num_bins, sizeof(long long));
        if (!global_bins) die("Allocation failed for global_bins.");
    }

    MPI_Reduce(local_bins, global_bins, num_bins, MPI_LONG_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Histogram (bin edges [%g, %g], %d bins):\n", min_val, max_val,
               num_bins);
        for (int b = 0; b < num_bins; b++) {
            double left = min_val + b * bin_width;
            double right = (b == num_bins - 1) ? max_val : left + bin_width;
            printf("[%g, %g)%s : %lld\n",
                   left,
                   right,
                   (b == num_bins - 1) ? "]" : "",
                   global_bins[b]);
        }
    }

    free(all_vals);
    free(counts);
    free(displs);
    free(local_vals);
    free(local_bins);
    free(global_bins);

    MPI_Finalize();
    return 0;
}
