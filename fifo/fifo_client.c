#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "fifo.h"

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

    // Comunication with server
    void *msg_buffer = malloc(args.msg_size);

    notify_server();

    for (; args.msg_count > 0; --args.msg_count)
    {
        wait_for_signal(&sig_action);

        if (fread(msg_buffer, args.msg_size, 1, fp) == 0)
        {
            sys_error("Error reading buffer");
        }

        notify_server();
    }

    free(msg_buffer);
    fclose(fp);

    return 0;
}