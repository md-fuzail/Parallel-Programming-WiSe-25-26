#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

/* Provided helper functions */

unsigned long my_rand(unsigned long* state, unsigned long lower, unsigned long upper) {
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    unsigned long result = (*state * 0x2545F4914F6CDD1DULL);
    unsigned long range = (upper > lower) ? (upper - lower) : 0UL;
    return (range > 0) ? (result % range + lower) : lower;
}

unsigned concatenate(unsigned x, unsigned y) {
    unsigned pow = 10;
    while (y >= pow)
        pow *= 10;
    return x * pow + y;
}

unsigned long hash(unsigned long x) {
    x ^= (x >> 21);
    x *= 2654435761UL;
    x ^= (x >> 13);
    x *= 2654435761UL;
    x ^= (x >> 17);
    return x;
}

/* Provided helper functions */


int main(int argc, char* argv[]) {

    if (argc != 10) {
        fprintf(stderr,
            "Usage: %s columns rows seed lower upper window_height verbose num_threads work_factor\n",
            argv[0]);
        return 1;
    }

    int columns       = atoi(argv[1]);
    int rows          = atoi(argv[2]);
    unsigned long seed = strtoul(argv[3], NULL, 10);
    unsigned long lower = strtoul(argv[4], NULL, 10);
    unsigned long upper = strtoul(argv[5], NULL, 10);
    int window_height = atoi(argv[6]);
    int verbose       = atoi(argv[7]);
    int num_threads   = atoi(argv[8]);
    int work_factor   = atoi(argv[9]);

    omp_set_num_threads(num_threads);

    printf("Starting heatmap_analysis\n");
    printf("Parameters: columns=%d, rows=%d, seed=%lu, lower=%lu, upper=%lu, "
           "window_height=%d, verbose=%d, num_threads=%d, work_factor=%d\n",
           columns, rows, seed, lower, upper,
           window_height, verbose, num_threads, work_factor);

    double start_time = omp_get_wtime();

    size_t total_cells = (size_t)rows * columns;

    // Allocate matrices on heap to avoid stack overflow
    unsigned long *A = malloc(total_cells * sizeof(unsigned long));
    unsigned long *B = malloc(total_cells * sizeof(unsigned long));

    if (!A || !B) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Initialize heatmap A
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            unsigned long s = seed * concatenate(i, j);
            A[i * columns + j] = my_rand(&s, lower, upper);
        }
    }

    #pragma omp parallel for schedule(static)
    for (size_t k = 0; k < total_cells; ++k) {
        unsigned long val = A[k];
        for (int w = 0; w < work_factor; ++w)
            val = hash(val);
        B[k] = val;
    }

    if (verbose) {
        printf("A:\n");
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                printf("%lu", A[i * columns + j]);
                if (j < columns - 1) printf(", ");
            }
            printf("\n");
        }
    }

    // Compute max sliding sums
    unsigned long *max_sums = malloc(columns * sizeof(unsigned long));

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < columns; ++j) {
        unsigned long window_sum = 0;
        for (int i = 0; i < window_height; ++i)
            window_sum += B[i * columns + j];
        unsigned long max_sum = window_sum;
        for (int i = 1; i <= rows - window_height; ++i) {
            window_sum = window_sum - B[(i - 1) * columns + j] + B[(i + window_height - 1) * columns + j];
            if (window_sum > max_sum)
                max_sum = window_sum;
        }
        max_sums[j] = max_sum;
    }

    if (verbose) {
        printf("Max sliding sums per column:\n");
        for (int j = 0; j < columns; ++j) {
            printf("%lu", max_sums[j]);
            if (j < columns - 1) printf(", ");
        }
        printf("\n");
    }

    int* hotspots_per_row = (int*) calloc(rows, sizeof(int));
    int total_hotspots = 0;

    #pragma omp parallel for reduction(+:total_hotspots) schedule(static)
    for (int i = 0; i < rows; ++i) {
        int row_count = 0;

        for (int j = 0; j < columns; ++j) {
            unsigned long center = B[i * columns + j];
            int is_hotspot = 1;

            if (i > 0 && center <= B[(i - 1) * columns + j])
                is_hotspot = 0;
            if (is_hotspot && i < rows - 1 && center <= B[(i + 1) * columns + j])
                is_hotspot = 0;
            if (is_hotspot && j > 0 && center <= B[i * columns + (j - 1)])
                is_hotspot = 0;
            if (is_hotspot && j < columns - 1 && center <= B[i * columns + (j + 1)])
                is_hotspot = 0;

            if (is_hotspot) {
                row_count++;
                total_hotspots++;
            }
        }
        hotspots_per_row[i] = row_count;
    }

    if (verbose) {
        printf("Hotspots per row:\n");
        for (int i = 0; i < rows; ++i)
            printf("Row %d: %d hotspot(s)\n", i, hotspots_per_row[i]);
    }

    printf("Total hotspots found: %d\n", total_hotspots);

    double end_time = omp_get_wtime();
    printf("Execution took %.4f s\n", end_time - start_time);

    free(hotspots_per_row);
    free(max_sums);
    free(A);
    free(B);

    return 0;
}
