#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 700
// #define THREADS 2
int main(int argc, char** argv) {
    int i, j, k;
    double sum = 0.0;

    /* allocate matrices dynamically as single contiguous blocks */
    double *a = (double*) malloc(sizeof(double) * N * N);
    double *b = (double*) malloc(sizeof(double) * N * N);
    double *c = (double*) malloc(sizeof(double) * N * N);

    if (!a || !b || !c) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* Init */
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            a[i*N + j] = 3.0 * i + j;
            b[i*N + j] = 5.2 * i + 2.3 * j;
            c[i*N + j] = 0.0;
        }
    }


    /* Matrix multiplication with parallel for-loop and timing */
    double t0 = omp_get_wtime();

    #pragma omp parallel for shared(a, b, c) private(i, j, k) schedule(static)
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            double tmp = 0.0;
            for (k = 0; k < N; ++k) {
                tmp += a[i*N + k] * b[k*N + j];
            }
            c[i*N + j] = tmp;
        }
    }

    double t1 = omp_get_wtime();
    double elapsed = t1 - t0;

    /* compute checksum */
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            sum += c[i*N + j];
        }
    }

    printf("Result (checksum): %f\n", sum);
    printf("Elapsed time (matrix multiply): %f seconds\n", elapsed);

    free(a);
    free(b);
    free(c);
    return 0;
}
