#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

static long num_steps = 100000;
double step;

int main() {
    int i;
    double step = 1.0 / (double) num_steps;

    int num_threads;
    double *partial_sums;

    /*********** BLOCK DISTRIBUTION ***********/
    double pi_block = 0.0;
    double start_block = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nt = omp_get_num_threads();

        #pragma omp single
        {
            num_threads = nt;
            partial_sums = (double*) malloc(nt * sizeof(double));
            for (int t = 0; t < nt; t++)
                partial_sums[t] = 0.0;
        }

        int chunk = num_steps / nt;
        int start_i = id * chunk;
        int end_i = (id == nt - 1) ? num_steps : start_i + chunk;

        double local_sum = 0.0;

        for (i = start_i; i < end_i; i++) {
            double x = (i + 0.5) * step;
            local_sum += 4.0 / (1.0 + x * x);
        }

        partial_sums[id] = local_sum;
    }

    for (int t = 0; t < num_threads; t++)
        pi_block += partial_sums[t] * step;

    double end_block = omp_get_wtime();


    /*********** CYCLIC DISTRIBUTION ***********/
    double pi_cyclic = 0.0;
    double start_cyclic = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nt = omp_get_num_threads();

        double local_sum = 0.0;

        /* CYCLIC WORK DISTRIBUTION */
        for (i = id; i < num_steps; i += nt) {
            double x = (i + 0.5) * step;
            local_sum += 4.0 / (1.0 + x * x);
        }

        partial_sums[id] = local_sum;
    }

    for (int t = 0; t < num_threads; t++)
        pi_cyclic += partial_sums[t] * step;

    double end_cyclic = omp_get_wtime();


    /*********** OUTPUT ***********/
    printf("\n--- BLOCK DISTRIBUTION ---\n");
    printf("Approximation of Pi: %.10f\n", pi_block);
    printf("Elapsed time: %f seconds\n", end_block - start_block);

    printf("\n--- CYCLIC DISTRIBUTION ---\n");
    printf("Approximation of Pi: %.10f\n", pi_cyclic);
    printf("Elapsed time: %f seconds\n", end_cyclic - start_cyclic);

    free(partial_sums);
    return 0;
}