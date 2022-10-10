#include "timer.h"
#include "omp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/**
 * @brief Sequential/OpenMP implementation of the Sieve of Erathostens
 *
 */

void usage(void)
{
    printf("Usage:\n");
    printf("./eratosthenes MAX\n");
    printf("\tWhere MAX: u64 maximum number\n");
}

static inline uint64_t square(uint64_t k)
{
    return k * k;
}

void mark(char *n_numbers, uint64_t max, uint64_t k)
{
    uint64_t kk = square(k);
    for (uint64_t i = kk; i <= max; i++)
    {
        if (i % k == 0)
            n_numbers[i] = true; // mark
    }
}

void find_smallest(const char *n_numbers, uint64_t max, uint64_t *k)
{
    uint64_t i = (*k) + 1;
    while (i != max)
    {
        if (!n_numbers[i])
        { // unmarked
            *k = i;
            break;
        }
        i++;
    }
}

void print_primes(const char *n_numbers, uint64_t max)
{
    if (max < 100)
    {
        for (uint64_t i = 0; i < max; i++)
        {
            if (!n_numbers[i])
                printf("%lu ", i);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    uint64_t max = 0;
    if (argc < 2)
    {
        usage();
        exit(0);
    }
    if (argc == 2)
    {
        max = strtoul(argv[1], NULL, 10);
    }
    printf("%lu\n", max);
    // Create a list of natural numbers 1..Max
    char *natural_numbers = (char *)calloc(max, sizeof(char)); // Linux will not allocate the memory until it's used
    // 0 (false) -> unmarked
    // 1 (true) -> marked
    memset(natural_numbers, false, max); // set all unmarked

    // BENCHMARK
    double start, end;
    GET_TIME(start);
    uint64_t k = 2;
#ifdef _OPENMP
    uint64_t sqrt_max = (uint64_t)sqrt(max);
    while (square(k) <= sqrt_max)
    {
        mark(natural_numbers, sqrt_max, k);
        find_smallest(natural_numbers, sqrt_max, &k);
    }
#pragma omp parallel for default(none) shared(natural_numbers, sqrt_max, max)
    for (uint64_t j = 2; j <= sqrt_max; j++)
    {
        if (!natural_numbers[j]) // unmarked
        {
                for (uint64_t i = sqrt_max + 1; i < max; i++)
                {
                    if (i % j == 0)
                        natural_numbers[i] = true;
                }
        }
    }

// #pragma omp parallel default(none) shared(natural_numbers, sqrt_max, max) firstprivate(k)
//     {
// #pragma omp for schedule(static) collapse(2)
//         for (uint64_t i = sqrt_max + 1; i < max; i++)
//         {
//             // printf("%d - i: %lu\n", omp_get_thread_num(), i);
//             for (uint64_t j = 2; j <= sqrt_max; j++)
//             {
//                 if (!natural_numbers[j]) // unmarked
//                 {
//                     if (i % j == 0)
//                         natural_numbers[i] = true;
//                 }
//             }
//         }
//     }
#else
    while (square(k) <= max)
    {
        mark(natural_numbers, max, k);
        find_smallest(natural_numbers, max, &k);
    }
#endif
    GET_TIME(end);
    printf("Elapsed: %lf\n", end - start);

    print_primes(natural_numbers, max);

    free(natural_numbers);
}
