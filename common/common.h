#ifndef BENCH_ARGS_H
#define BENCH_ARGS_H

#include <stddef.h>

typedef struct benc_args
{
    size_t msg_size;
    size_t msg_count;

} bench_args;

typedef struct bench_results
{
    unsigned long long total_start;
    unsigned long long iteration_start;
    unsigned long long min_time;
    unsigned long long max_time;
    unsigned long long sum;
    unsigned long long squared_sum;
} bench_results;

void get_bench_args(bench_args *args, int argc, char **argv);

void sys_error(const char *msg);

unsigned long long now();
void init_benchmakr(bench_results *bench);
void benchmark(bench_results *bench);
void evaluate_benchmark(bench_results *bench, bench_args *args);

#endif