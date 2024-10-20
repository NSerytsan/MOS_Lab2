#include <fcntl.h>
#include <unistd.h>
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

    int fd = open(FIFO_BENCH_FILE, O_RDONLY);
    if (fd == -1)
    {
        sys_error("Error opening file to FIFO on client-side");
    }

    void *msg = malloc(args.msg_size);

    notify_server();
    const int fifo_sz = fcntl(fd, F_GETPIPE_SZ);

    if (args.msg_count == 0)
    {
        wait_for_signal(&sig_action);
        bench_rw_results results;
        results.start = now_us();
        ssize_t total_read = 0;
        while (total_read < args.msg_size)
        {
            ssize_t read_bytes = read(fd, msg, fifo_sz);

            if (read_bytes == -1)
            {
                sys_error("Error reading buffer");
            }

            total_read += read_bytes;
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

            ssize_t total_read = 0;
            while (total_read < args.msg_size)
            {
                ssize_t read_bytes = read(fd, msg, fifo_sz);

                if (read_bytes == -1)
                {
                    sys_error("Error reading buffer");
                }

                total_read += read_bytes;
            }

            notify_server();
        }
    }

    free(msg);
    close(fd);

    return 0;
}