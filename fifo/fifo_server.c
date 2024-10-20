#include <unistd.h>
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

    int fd = open(FIFO_BENCH_FILE, O_WRONLY);
    if (fd == -1)
    {
        sys_error("Error opening FIFO");
    }

    void *msg = malloc(args.msg_size);

    wait_for_signal(&sig_action);

    const int fifo_sz = fcntl(fd, F_GETPIPE_SZ);

    if (args.msg_count == 0)
    {
        bench_rw_results results;
        notify_client();
        results.start = now_us();
        ssize_t total_write = 0;

        while (total_write < args.msg_size)
        {
            ssize_t write_bytes = write(fd, msg, (args.msg_size - total_write > fifo_sz) ? fifo_sz : args.msg_size - total_write);

            if (write_bytes == -1)
            {
                sys_error("Error writing buffer");
            }
            total_write += write_bytes;
        }

        results.end = now_us();
        FILE *fp = fopen(FIFO_SERVER_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        bench_results results;
        init_benchmark(&results);

        for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
        {
            results.iteration_start = now();
            notify_client();
            ssize_t total_write = 0;
            while (total_write < args.msg_size)
            {
                ssize_t write_bytes = write(fd, msg, (args.msg_size - total_write > fifo_sz) ? fifo_sz : args.msg_size - total_write);

                if (write_bytes == -1)
                {
                    sys_error("Error writing buffer");
                }
                total_write += write_bytes;
            }

            wait_for_signal(&sig_action);

            benchmark(&results);
        }
        evaluate_benchmark(&results, &args, NULL);
    }

    free(msg);
    close(fd);

    if (remove(FIFO_BENCH_FILE) == -1)
    {
        sys_error("Error removing FIFO");
    }

    return 0;
}