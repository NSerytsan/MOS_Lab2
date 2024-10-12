#ifndef BENCH_ARGS_H
#define BENCH_ARGS_H

#include <stddef.h>

typedef struct benc_args
{
    size_t msg_size;
    size_t msg_count;

} bench_args;

void get_bench_args(bench_args *args, int argc, char **argv);

void sys_error(const char *msg);

#endif