#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"

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

void sys_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}