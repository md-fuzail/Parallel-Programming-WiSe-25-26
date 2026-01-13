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

    /* ----------------- STEP 3: Part 1 – Sliding window sums ----------------- */
    unsigned long* max_sums = (unsigned long*) calloc(columns, sizeof(unsigned long));

    #pragma omp parallel for schedule(static)
    for (int col = 0; col < columns; ++col) {
        unsigned long max_sum = 0;

        for (int start = 0; start <= rows - window_height; ++start) {
            unsigned long sum = 0;

            for (int r = start; r < start + window_height; ++r) {
                unsigned long val = A[r * columns + col];
                for (int w = 0; w < work_factor; ++w)
                    val = hash(val);
                sum += val;
            }

            if (sum > max_sum)
                max_sum = sum;
        }
        max_sums[col] = max_sum;
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
    int* hotspots_per_row = (int*) calloc(rows, sizeof(int));
    int total_hotspots = 0;

    #pragma omp parallel for reduction(+:total_hotspots) schedule(static)
    for (int i = 0; i < rows; ++i) {
        int row_count = 0;

        for (int j = 0; j < columns; ++j) {

            unsigned long center = A[i * columns + j];
            for (int w = 0; w < work_factor; ++w)
                center = hash(center);

            int is_hotspot = 1;

            if (i > 0) {
                unsigned long up = A[(i - 1) * columns + j];
                for (int w = 0; w < work_factor; ++w) up = hash(up);
                if (center <= up) is_hotspot = 0;
            }
            if (i < rows - 1) {
                unsigned long down = A[(i + 1) * columns + j];
                for (int w = 0; w < work_factor; ++w) down = hash(down);
                if (center <= down) is_hotspot = 0;
            }
            if (j > 0) {
                unsigned long left = A[i * columns + j - 1];
                for (int w = 0; w < work_factor; ++w) left = hash(left);
                if (center <= left) is_hotspot = 0;
            }
            if (j < columns - 1) {
                unsigned long right = A[i * columns + j + 1];
                for (int w = 0; w < work_factor; ++w) right = hash(right);
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
    free(max_sums);
    free(hotspots_per_row);

    return 0;
}