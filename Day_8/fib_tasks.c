/* fib_tasks.c
 *
 * Usage:
 *   ./fib_tasks <n> <cutoff>
 *
 * Example:
 *   ./fib_tasks 40 10
 *
 * If arguments are omitted, defaults are n=40, cutoff=10.
 *
 * This program:
 *  - computes fib(n) sequentially
 *  - computes fib(n) with OpenMP tasks and a cutoff
 *  - prints times and thread information
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Sequential recursive Fibonacci (intentionally naive) */
long fib_seq(int n) {
    if (n < 2) return n;
    return fib_seq(n-1) + fib_seq(n-2);
}

/* Task-based recursive Fibonacci with cutoff.
 * If n <= cutoff, compute sequentially to avoid task overhead.
 *
 * This function must be called from inside a parallel region
 * (or by a task) so that #pragma omp task actually creates tasks.
 */
long fib_task(int n, int cutoff) {
    if (n < 2) return n;
    if (n <= cutoff) {
        return fib_seq(n);
    }

    long x = 0, y = 0;

    /* create task for fib(n-1) */
    #pragma omp task firstprivate(n, cutoff) shared(x)
    {
        x = fib_task(n-1, cutoff);
    }

    /* create task for fib(n-2) */
    #pragma omp task firstprivate(n, cutoff) shared(y)
    {
        y = fib_task(n-2, cutoff);
    }

    /* wait for both child tasks to finish */
    #pragma omp taskwait

    return x + y;
}

int main(int argc, char **argv) {
    int n = 40;            /* default Fibonacci input */
    int cutoff = 10;       /* default cutoff for task creation */

    if (argc >= 2) n = atoi(argv[1]);
    if (argc >= 3) cutoff = atoi(argv[2]);

    printf("Fibonacci n = %d, cutoff = %d\n", n, cutoff);

    /* Sequential run */
    double t0 = omp_get_wtime();
    long seq = fib_seq(n);
    double t1 = omp_get_wtime();
    double seq_time = t1 - t0;
    printf("Sequential: fib(%d) = %ld, time = %.6f s\n", n, seq, seq_time);

    /* Parallel run using tasks.
     * We measure the whole region time. We will also print how many threads are active.
     */
    long par_result = 0;
    double par_start = omp_get_wtime();

    #pragma omp parallel
    {
        /* Print actual number of threads once (thread 0) */
        #pragma omp single
        {
            int used = omp_get_num_threads();
            printf("Running parallel tasks with %d thread(s)\n", used);
        }

        /* Only one thread should spawn the initial tasks (single) */
        #pragma omp single
        {
            /* spawn the first task(s) using the task-based function */
            par_result = fib_task(n, cutoff);
        }
        /* implicit barrier here: all threads wait until tasks complete and single finishes */
    }

    double par_end = omp_get_wtime();
    double par_time = par_end - par_start;

    printf("Parallel (tasks): fib(%d) = %ld, time = %.6f s\n", n, par_result, par_time);

    /* Speedup and correctness check */
    double speedup = seq_time / par_time;
    printf("Speedup (seq / par): %.3f\n", speedup);

    /* Simple correctness check */
    if (seq != par_result) {
        fprintf(stderr, "ERROR: results do not match! seq=%ld par=%ld\n", seq, par_result);
        return 1;
    }

    return 0;
}