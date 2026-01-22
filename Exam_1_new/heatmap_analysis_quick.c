#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* provided helper functions */

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


int main(int argc, char* argv[]) {

    if (argc != 10) {
        fprintf(stderr,
            "Usage: %s columns rows seed lower upper window_height verbose num_threads work_factor\n",
            argv[0]);
        return 1;
    }

    /* Parse command line arguments */
    int columns = atoi(argv[1]);
    int rows = atoi(argv[2]);
    unsigned long seed = strtoul(argv[3], NULL, 10);
    unsigned long lower = strtoul(argv[4], NULL, 10);
    unsigned long upper = strtoul(argv[5], NULL, 10);
    int window_height = atoi(argv[6]);
    int verbose = atoi(argv[7]);
    int num_threads = atoi(argv[8]);
    int work_factor = atoi(argv[9]);

    /* Setup OpenMP environment */
    omp_set_num_threads(num_threads);

    printf("Starting heatmap_analysis\n");
    printf("Parameters: columns=%d, rows=%d, seed=%lu, lower=%lu, upper=%lu, "
           "window_height=%d, verbose=%d, num_threads=%d, work_factor=%d\n",
           columns, rows, seed, lower, upper,
           window_height, verbose, num_threads, work_factor);

    double start_time = omp_get_wtime();

    /* Allocate memory for heatmap and hashed copy */
    unsigned long* A = malloc((size_t)rows * columns * sizeof(unsigned long));
    unsigned long* B = malloc((size_t)rows * columns * sizeof(unsigned long));

    /* Md Fuzail */
    /* initialize heatmap with random values */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            unsigned long s = seed * concatenate(i, j);
            A[i * columns + j] = my_rand(&s, lower, upper);
        }
    }

    /* Ehtesham Faraz */
    /* Apply hash transformation */
    #pragma omp parallel for schedule(static)
    for (int k = 0; k < rows * columns; ++k) {
        unsigned long v = A[k];
        for (int w = 0; w < work_factor; ++w)
            v = hash(v);
        B[k] = v;
    }

    /* Together start */
    /* Early Exit case */
    int early_exit = 0;
    int bad_row = -1;

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < rows; ++i) {

            /* Check if cancellation was triggered by another thread */
            #pragma omp cancellation point for

            int row_hotspots = 0;

            /* Check row for hotspots */
            for (int j = 0; j < columns; ++j) {
                unsigned long center = B[i * columns + j];
                int is_hotspot = 1;

                if (i > 0 && center <= B[(i - 1) * columns + j]) is_hotspot = 0;
                if (is_hotspot && i < rows - 1 && center <= B[(i + 1) * columns + j]) is_hotspot = 0;
                if (is_hotspot && j > 0 && center <= B[i * columns + j - 1]) is_hotspot = 0;
                if (is_hotspot && j < columns - 1 && center <= B[i * columns + j + 1]) is_hotspot = 0;

                if (is_hotspot)
                    row_hotspots++;
            }

            /* If no hotspots found, trigger early exit */
            if (row_hotspots == 0) {
                #pragma omp critical
                {
                    if (!early_exit) {
                        early_exit = 1;
                        bad_row = i;
                        printf("Row %d contains no hotspots.\n", i);
                        printf("Early exit.\n");
                    }
                }
                /* Signal cancellation to other threads */
                #pragma omp cancel for
            }
        }
    }

    /* Handle early exit case */
    if (early_exit) {
        double end_time = omp_get_wtime();
        printf("Execution took %.4f s\n", end_time - start_time);

        free(A);
        free(B);
        return 0;
    }
    /* Together end */

    /* Full Analysis: (Success Case) */
    /* Md Fuzail */
    /* Sliding window sums */
    unsigned long* max_sums = malloc(columns * sizeof(unsigned long));

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < columns; ++j) {
        unsigned long sum = 0;
        for (int i = 0; i < window_height; ++i)
            sum += B[i * columns + j];

        unsigned long max_sum = sum;

        for (int i = 1; i <= rows - window_height; ++i) {
            sum -= B[(i - 1) * columns + j];
            sum += B[(i + window_height - 1) * columns + j];
            if (sum > max_sum)
                max_sum = sum;
        }
        max_sums[j] = max_sum;
    }

    if (verbose) {
        printf("Max sliding sums per column:\n");
        for (int j = 0; j < columns; ++j)
            printf("%lu%s", max_sums[j], (j + 1 < columns) ? ", " : "\n");
    }

    /* Full hotspot counting */
    /* Ehtesham Faraz */
    int* hotspots_per_row = calloc(rows, sizeof(int));
    int total_hotspots = 0;

    #pragma omp parallel for reduction(+:total_hotspots) schedule(static)
    for (int i = 0; i < rows; ++i) {
        int count = 0;

        for (int j = 0; j < columns; ++j) {
            unsigned long c = B[i * columns + j];
            int ok = 1;

            if (i > 0 && c <= B[(i - 1) * columns + j]) ok = 0;
            if (ok && i < rows - 1 && c <= B[(i + 1) * columns + j]) ok = 0;
            if (ok && j > 0 && c <= B[i * columns + j - 1]) ok = 0;
            if (ok && j < columns - 1 && c <= B[i * columns + j + 1]) ok = 0;

            if (ok) {
                count++;
                total_hotspots++;
            }
        }
        hotspots_per_row[i] = count;
    }

    if (verbose) {
        printf("Hotspots per row:\n");
        for (int i = 0; i < rows; ++i)
            printf("Row %d: %d hotspot(s)\n", i, hotspots_per_row[i]);
    }

    printf("Total hotspots found: %d\n", total_hotspots);

    double end_time = omp_get_wtime();
    printf("Execution took %.4f s\n", end_time - start_time);
    free(A);
    free(B);
    free(max_sums);
    free(hotspots_per_row);

    return 0;
}