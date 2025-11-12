#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main(int argc, char **argv)
{
#pragma omp parallel
    printf("I am parallel \n");
    printf("I am not parallel \n");
}

// 4. Modify program so both print statements are executed by all threads

// Use braces {} to include both printfs inside the parallel region:

// #include <stdio.h>
// #include <omp.h>

// int main() {
//     #pragma omp parallel
//     {
//         printf("I am parallel\n");
//         printf("I am also parallel\n");
//     }
// }

// 5. Print thread ID and total number of threads

// #include <stdio.h>
// #include <omp.h>

// int main() {
//     #pragma omp parallel
//     {
//         int tid = omp_get_thread_num();
//         int nthreads = omp_get_num_threads();

//         printf("Thread %d of %d: I am parallel\n", tid, nthreads);
//         printf("Thread %d of %d: I am also parallel\n", tid, nthreads);
//     }

//     // Outside the parallel region
//     printf("Outside parallel region: Number of threads = %d\n", omp_get_num_threads());

//     return 0;
// }