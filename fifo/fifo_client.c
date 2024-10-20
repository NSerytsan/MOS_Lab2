#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "fifo_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    struct sigaction sig_action;
    setup_client_signals(&sig_action);

    wait_for_signal(&sig_action);

    FILE *fp = fopen(FIFO_BENCH_FILE, "r");
    if (fp == NULL)
    {
        sys_error("Error opening stream to FIFO on client-side");
    }

    // Communication with server
    void *msg = malloc(args.msg_size);

    notify_server();

    if (args.msg_count == 0)
    {
        wait_for_signal(&sig_action);
        bench_rw_results results;
        results.start = now_us();
        if (fread(msg, args.msg_size, 1, fp) == 0)
        {
            sys_error("Error reading buffer");
        }
        results.end = now_us();
        FILE *fp = fopen(FIFO_CLIENT_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        for (; args.msg_count > 0; args.msg_count--)
        {
            wait_for_signal(&sig_action);

            if (fread(msg, args.msg_size, 1, fp) == 0)
            {
                sys_error("Error reading buffer");
            }

            notify_server();
        }
    }

    free(msg);
    fclose(fp);

    return 0;
}