#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000000  // size of the array

int main(int argc, char** argv) {
    double *a;
    a = (double*) malloc(N * sizeof(double));
    if (a == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize the array with random values
    srand(0); // fixed seed for reproducibility
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100; // values between 0 and 99
    }

    // ===== Parallel For Version =====
    double sum_parallel_for = 0.0;
    double start = omp_get_wtime();

    #pragma omp parallel for reduction(+:sum_parallel_for)
    for (int i = 0; i < N; i++) {
        sum_parallel_for += a[i];
    }

    double end = omp_get_wtime();
    double time_parallel_for = end - start;

    printf("Parallel For: Sum = %.2f, Time = %f s\n", sum_parallel_for, time_parallel_for);

    // ===== Taskloop Version =====
    double sum_taskloop = 0.0;
    int grainsize = 10000;

    start = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:sum_taskloop) grainsize(grainsize)
            for (int i = 0; i < N; i++) {
                sum_taskloop += a[i];
            }
        }
    }

    end = omp_get_wtime();
    double time_taskloop = end - start;

    printf("Taskloop: Sum = %.2f, Time = %f s (grainsize=%d)\n", sum_taskloop, time_taskloop, grainsize);

    free(a);
    return 0;
}