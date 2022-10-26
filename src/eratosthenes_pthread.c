#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

/**
 * @brief Pthread implementation of the Sieve of Eratosthens
 *
 */

const char *TAG = "Pthread";

void usage(void)
{
    printf("[%s] Usage:\n", TAG);
    printf("./eratosthenes MAX N\n");
    printf("\tWhere:\n");
    printf("\t\tMAX: u64 maximum number\n");
    printf("\t\tN: number of threads\n");
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
    else
    {
        int count = 0;
        for (uint64_t i = 0; i < max; i++)
        {
            if (!n_numbers[i])
                count++;
        }
        printf("prime count: %d\n", count);
    }
}

typedef struct
{
    char *n_numbers; // pointer to natural numbers buffer
    uint64_t max;    // natural number buffer dimension
    uint64_t start;  // start of the buffer where pthread operate
    uint64_t end;    // end of the buffer where pthread operate
    size_t id;
} th_data;

void *mark_chunk(void *parameters)
{
    th_data *data = (th_data *)parameters;
    uint64_t sqrt_max = (uint64_t)sqrt(data->max);
    for (uint64_t j = 2; j <= sqrt_max; j++)
    {
        if (!data->n_numbers[j]) // unmarked
        {
            for (uint64_t i = data->start; i <= (data->end > data->max ? data->max : data->end); i++)
            {
                if (i % j == 0)
                    data->n_numbers[i] = true;
            }
        }
    }
    free(data);
    return NULL;
}

int main(int argc, char *argv[])
{
    uint64_t max = 0;
    int n_threads = 1;
    if (argc < 3)
    {
        usage();
        exit(0);
    }
    if (argc == 3)
    {
        max = strtoul(argv[1], NULL, 10);
        n_threads = (int)strtol(argv[2], NULL, 10);
    }
    printf("%lu\n", max);
    // Create a list of natural numbers 1..Max
    char *natural_numbers = (char *)calloc(max + 1, sizeof(char)); // Linux will not allocate the memory until it's used
    // 0 (false) -> unmarked
    // 1 (true) -> marked
    memset(natural_numbers, false, max); // set all unmarked

    // Prepare the pthreads
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * n_threads);
    uint64_t k = 2;
    uint64_t sqrt_max = (uint64_t)sqrt(max);
    int chunk = (max - sqrt_max) / n_threads;
    int remaining = max % n_threads;

    // BENCHMARK
    double start, end;
    GET_TIME(start);
    while (square(k) <= sqrt_max)
    {
        mark(natural_numbers, sqrt_max, k);
        find_smallest(natural_numbers, sqrt_max, &k);
    }
    // Pthread part
    int next_start = sqrt_max + 1;
    for (size_t id = 0; id < n_threads; id++)
    {
        th_data *data = (th_data *)calloc(sizeof(th_data), 1);
        data->n_numbers = natural_numbers;
        data->max = max;
        data->start = next_start;
        data->end = next_start + (chunk) + (remaining-- > 0 ? 1 : 0);
        data->id = id;
        pthread_create(&threads[id], NULL, mark_chunk, (void*)data);
        next_start = data->end;
    }
    // Wait the threads finish
    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    GET_TIME(end);
    printf("Elapsed: %lf\n", end - start);
    free(threads);

    print_primes(natural_numbers, max);

    free(natural_numbers);
}
