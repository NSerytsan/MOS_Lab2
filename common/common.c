#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#include "common.h"

// Input args routines
void usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s -s <message size> -c <message count>\n", prog_name);
    exit(EXIT_FAILURE);
}

void get_bench_args(bench_args *args, int argc, char **argv)
{
    if (args == NULL)
        return;
    int opt = 0;

    args->msg_count = 0;
    args->msg_size = 0;
    while (((opt = getopt(argc, argv, "hc:s:"))))
    {
        switch (opt)
        {
        case -1:
            return;
        case 'c':
            args->msg_count = atoi(optarg);
            break;
        case 's':
            args->msg_size = atoi(optarg);
            break;
        case 'h':
        default:
            usage(argv[0]);
            break;
        }
    }
}

// Error routines
void sys_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// Benchmark routines
unsigned long long now()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void init_benchmakr(bench_results *bench)
{
    if (bench == NULL)
        return;
    bench->min_time = INT32_MAX;
    bench->max_time = 0;
    bench->sum = 0;
    bench->squared_sum = 0;
    bench->total_start = now();
}

void benchmark(bench_results *bench)
{
    const unsigned long long time = now() - bench->iteration_start;
    if (time < bench->min_time)
    {
        bench->min_time = time;
    }

    if (time > bench->max_time)
    {
        bench->max_time = time;
    }

    bench->sum += time;
    bench->squared_sum += (time * time);
}

void evaluate_benchmark(bench_results *bench, bench_args *args)
{
    const unsigned long total_time = now() - bench->total_start;
    const double average = ((double)bench->sum) / args->msg_count;

    double sigma = bench->squared_sum / args->msg_count;
    sigma = sqrt(sigma - (average * average));

    int message_rate = (int)(args->msg_count / (total_time / 1e9));

    printf("\nMessage size:       %lu\n", args->msg_size);
    printf("Message count:      %lu\n", args->msg_count);
    printf("Total duration:     %.3f\tms\n", total_time / 1e6);
    printf("Average duration:   %.3f\tus\n", average / 1000.0);
    printf("Minimum duration:   %.3f\tus\n", bench->min_time / 1000.0);
    printf("Maximum duration:   %.3f\tus\n", bench->max_time / 1000.0);
    printf("Standard deviation: %.3f\tus\n", sigma / 1000.0);
    printf("Message rate:       %d\tmsg/s\n", message_rate);
}