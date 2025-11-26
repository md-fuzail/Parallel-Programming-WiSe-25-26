#include <stdio.h>
#include <omp.h>

static long num_steps = 100000;
double step;

int main() {
    int i;
    double x, pi, value, sum = 0.0;
    step = 1.0 / (double) num_steps;

    double start = omp_get_wtime();

    #pragma omp parallel for schedule(runtime) reduction(+:sum)
    for (i = 0; i < num_steps; i++) {
        x = (i + 0.5) * step;
        double value = 4.0 / (1.0 + x * x);
        
        #pragma omp atomic
        sum += value;
    }

    pi = step * sum;
    double end = omp_get_wtime();

    printf("Pi (static): %.10f\n", pi);
    printf("Elapsed time: %f seconds\n", end - start);

    return 0;
}
