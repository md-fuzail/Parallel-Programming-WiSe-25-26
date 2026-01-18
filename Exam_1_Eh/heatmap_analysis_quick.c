#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* =========================================================
   Provided helper functions
   ========================================================= */

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

/* =========================================================
   MAIN PROGRAM — Task 1.2
   ========================================================= */

int main(int argc, char* argv[]) {

    /* ----------------- Argument parsing ----------------- */
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

    /* ----------------- Allocate heatmap ----------------- */
    unsigned long* A = malloc((size_t) rows * columns * sizeof(unsigned long));
    unsigned long* B = malloc((size_t) rows * columns * sizeof(unsigned long));

    if (!A || !B) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    /* ----------------- STEP 1: Initialization ----------------- */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            unsigned long s = seed * concatenate(i, j);
            A[i * columns + j] = my_rand(&s, lower, upper);
        }
    }

    /* ----------------- STEP 2: Hash precomputation ----------------- */
    #pragma omp parallel for schedule(static)
    for (int k = 0; k < rows * columns; ++k) {
        unsigned long val = A[k];
        for (int w = 0; w < work_factor; ++w)
            val = hash(val);
        B[k] = val;
    }

    /* ----------------- STEP 3: Early hotspot detection ----------------- */
    int early_exit = 0;
    int failing_row = -1;

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < rows; ++i) {

        if (early_exit)
            continue;

        int has_hotspot = 0;

        for (int j = 0; j < columns; ++j) {

            unsigned long center = B[i * columns + j];
            int is_hotspot = 1;

            if (i > 0 && center <= B[(i - 1) * columns + j]) is_hotspot = 0;
            if (i < rows - 1 && center <= B[(i + 1) * columns + j]) is_hotspot = 0;
            if (j > 0 && center <= B[i * columns + (j - 1)]) is_hotspot = 0;
            if (j < columns - 1 && center <= B[i * columns + (j + 1)]) is_hotspot = 0;

            if (is_hotspot) {
                has_hotspot = 1;
                break;
            }
        }

        if (!has_hotspot) {
            #pragma omp critical
            {
                if (!early_exit) {
                    early_exit = 1;
                    failing_row = i;
                    printf("Row %d contains no hotspots.\n", i);
                    printf("Early exit.\n");
                }
            }
        }
    }

    /* ----------------- SUCCESS CASE: Full analysis ----------------- */
    if (!early_exit) {

        unsigned long *max_sums = malloc(columns * sizeof(unsigned long));

        #pragma omp parallel for schedule(static)
        for (int j = 0; j < columns; ++j) {

            unsigned long window_sum = 0;

            for (int i = 0; i < window_height; ++i)
                window_sum += B[i * columns + j];

            unsigned long max_sum = window_sum;

            for (int i = 1; i <= rows - window_height; ++i) {
                window_sum = window_sum
                    - B[(i - 1) * columns + j]
                    + B[(i + window_height - 1) * columns + j];

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

        free(max_sums);
    }

    double end_time = omp_get_wtime();
    printf("Execution took %.4f s\n", end_time - start_time);

    free(A);
    free(B);
    return 0;
}