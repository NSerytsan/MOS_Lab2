#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "fifo_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    struct sigaction sig_action;
    setup_server_signals(&sig_action);

    if (mkfifo(FIFO_BENCH_FILE, 0666) < 0)
    {
        sys_error("Error creating FIFO");
    }

    notify_client();

    FILE *fp = fopen(FIFO_BENCH_FILE, "w");
    if (fp == NULL)
    {
        sys_error("Error opening FIFO");
    }

    // Communication with client
    void *msg = malloc(args.msg_size);

    bench_results results;
    init_benchmark(&results);

    wait_for_signal(&sig_action);

    for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
    {
        results.iteration_start = now();
        if (fwrite(msg, args.msg_size, 1, fp) == 0)
        {
            sys_error("Error writing buffer");
        }

        fflush(fp);

        notify_client();
        wait_for_signal(&sig_action);

        benchmark(&results);
    }

    evaluate_benchmark(&results, &args, NULL);

    free(msg);
    fclose(fp);
    if (remove(FIFO_BENCH_FILE) == -1)
    {
        sys_error("Error removing FIFO");
    }

    return 0;
}