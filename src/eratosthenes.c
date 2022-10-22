#include "timer.h"
#include "omp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/**
 * @brief Sequential implementation of the Sieve of Erathostens
 *
 */

const char *TAG = "Sequential";

void usage(void)
{
    printf("[%s] Usage:\n", TAG);
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
    if (max <= 100)
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
    while (square(k) <= max)
    {
        mark(natural_numbers, max, k);
        find_smallest(natural_numbers, max, &k);
    }
    GET_TIME(end);
    printf("Elapsed: %lf\n", end - start);

    print_primes(natural_numbers, max);

    free(natural_numbers);
}
