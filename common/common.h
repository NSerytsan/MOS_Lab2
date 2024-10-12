#ifndef BENCH_ARGS_H
#define BENCH_ARGS_H

#include <stddef.h>
#include <signal.h>

#define IGNORE_USR1 0x0
#define IGNORE_USR2 0x0
#define BLOCK_USR1 0x1
#define BLOCK_USR2 0x2

#define WAIT 0x0
#define NOTIFY 0x1

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

void setup_signals(struct sigaction *signal_action, int flags);
void setup_client_signals(struct sigaction *signal_action);
void wait_for_signal(struct sigaction *signal_action);
void notify_server();
void notify_client();

#endif