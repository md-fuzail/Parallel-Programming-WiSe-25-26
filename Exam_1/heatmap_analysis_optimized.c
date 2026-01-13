#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
   MAIN PROGRAM
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

    /* ----------------- Start timing ----------------- */
    double start_time = omp_get_wtime();

    /* ----------------- Allocate heatmap ----------------- */
    unsigned long* A = (unsigned long*) malloc((size_t)rows * columns * sizeof(unsigned long));
    if (!A) {
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
    
    /* ----------------- STEP 2: Pre-compute hashed values ----------------- */
    unsigned long *B = (unsigned long*) malloc(rows * columns * sizeof(unsigned long));
    #pragma omp parallel for schedule(static)
    for (int k = 0; k < rows * columns; ++k) {
        unsigned long val = A[k];
        for (int w = 0; w < work_factor; ++w) {
            val = hash(val);
        }
        B[k] = val;
    }

    /* ----------------- STEP 3: Part 1 – Sliding window sums ----------------- */
    unsigned long *max_sums =
        (unsigned long*) malloc(columns * sizeof(unsigned long));

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < columns; ++j) {

        unsigned long window_sum = 0;

        /* Initial window */
        for (int i = 0; i < window_height; ++i) {
            unsigned long val = B[i * columns + j];
            for (int w = 0; w < work_factor; ++w)
                val = hash(val);
            window_sum += val;
        }

        unsigned long max_sum = window_sum;

        /* Slide window */
        for (int i = 1; i <= rows - window_height; ++i) {

            unsigned long out_val = B[(i - 1) * columns + j];
            unsigned long in_val  = B[(i + window_height - 1) * columns + j];

            for (int w = 0; w < work_factor; ++w) {
                out_val = hash(out_val);
                in_val  = hash(in_val);
            }

            window_sum = window_sum - out_val + in_val;

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

    /* ----------------- STEP 4: Part 2 – Local hotspots ----------------- */
    
    int *hotspots_per_row = (int*) calloc(rows, sizeof(int));
    int total_hotspots = 0;

    #pragma omp parallel for reduction(+:total_hotspots) schedule(static)
    for (int i = 0; i < rows; ++i) {
        int row_count = 0;

        for (int j = 0; j < columns; ++j) {
            // Read directly from the pre-computed buffer B
            unsigned long center = B[i * columns + j];
            int is_hotspot = 1;

            /* Up */
            if (i > 0) {
                unsigned long up = B[(i - 1) * columns + j]; 
                if (center <= up) is_hotspot = 0;
            }

            /* Down (Only check if not already disqualified) */
            if (is_hotspot && i < rows - 1) {
                unsigned long down = B[(i + 1) * columns + j];
                if (center <= down) is_hotspot = 0;
            }

            /* Left */
            if (is_hotspot && j > 0) {
                unsigned long left = B[i * columns + (j - 1)];
                if (center <= left) is_hotspot = 0;
            }

            /* Right */
            if (is_hotspot && j < columns - 1) {
                unsigned long right = B[i * columns + (j + 1)];
                if (center <= right) is_hotspot = 0;
            }

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

    /* ----------------- End timing ----------------- */
    double end_time = omp_get_wtime();
    printf("Execution took %.4f s\n", end_time - start_time);

    /* ----------------- Cleanup ----------------- */
    free(A);
    free(B);
    free(max_sums);
    free(hotspots_per_row);

    return 0;
}