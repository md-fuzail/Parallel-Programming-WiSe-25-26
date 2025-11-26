#include <stdio.h>
#include <omp.h>

static long num_steps = 100000;
double step;

int main() {
    int i;
    double pi = 0.0;
    step = 1.0 / (double) num_steps;

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nt = omp_get_num_threads();

        int chunk = num_steps / nt;
        int start_i = id * chunk;
        int end_i = (id == nt - 1) ? num_steps : start_i + chunk;

        double local_sum = 0.0;

        for (i = start_i; i < end_i; i++) {
            double x = (i + 0.5) * step;
            local_sum += 4.0 / (1.0 + x * x);
        }

        // ATOMIC UPDATE
        #pragma omp atomic
        pi += local_sum * step;
    }

    double end = omp_get_wtime();

    printf("Approximation of Pi (atomic): %.10f\n", pi);
    printf("Elapsed time: %f seconds\n", end - start);

    return 0;
}