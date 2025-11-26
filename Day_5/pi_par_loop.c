#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

static long num_steps = 100000;
double step;

int main() {
    int i;
    double x, pi, value, sum = 0.0;
    step = 1.0 / (double) num_steps;

    omp_sched_t kind;
    int chunk;
    omp_get_schedule(&kind, &chunk);

    printf("OMP_SCHEDULE from environment: %s\n", getenv("OMP_SCHEDULE"));

    const char *schedule_name;
    switch (kind) {
        case omp_sched_static:   schedule_name = "static"; break;
        case omp_sched_dynamic:  schedule_name = "dynamic"; break;
        case omp_sched_guided:   schedule_name = "guided"; break;
        case omp_sched_auto:     schedule_name = "auto"; break;
        default:                  schedule_name = "unknown"; break;
    }

    printf("Schedule being used: %s, Chunk size: %d\n", schedule_name, chunk);

    double start = omp_get_wtime();

    #pragma omp parallel for schedule(runtime)
    for (i = 0; i < num_steps; i++) {
        x = (i + 0.5) * step;
        double value = 4.0 / (1.0 + x * x);

        #pragma omp atomic
        sum += value;
    }

    pi = step * sum;
    double end = omp_get_wtime();

    printf("Pi: %.10f\n", pi);
    printf("Elapsed time: %f seconds\n", end - start);

    return 0;
}