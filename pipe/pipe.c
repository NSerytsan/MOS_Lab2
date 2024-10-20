#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/common.h"

#define PIPE_CLIENT_OUT "./pipe_client.out"
#define PIPE_SERVER_OUT "./pipe_server.out"

void child_flow(int fds[2], bench_args *args)
{
    close(fds[1]);

    struct sigaction sig_action;
    setup_client_signals(&sig_action);
    void *msg = malloc(args->msg_size);
    const int fifo_sz = fcntl(fds[0], F_GETPIPE_SZ);

    notify_server();
    if (args->msg_count == 0)
    {
        wait_for_signal(&sig_action);
        bench_rw_results results;
        results.start = now_us();
        ssize_t total_read = 0;
        while (total_read < args->msg_size)
        {
            ssize_t read_bytes = read(fds[0], msg, fifo_sz);

            if (read_bytes == -1)
            {
                sys_error("Error reading from pipe");
            }

            total_read += read_bytes;
        }
        results.end = now_us();
        FILE *fp = fopen(PIPE_CLIENT_OUT, "w");
        evaluate_rw_benchmark(&results, args, fp);
        fclose(fp);
    }
    else
    {
        for (; args->msg_count > 0; --args->msg_count)
        {
            wait_for_signal(&sig_action);

            ssize_t total_read = 0;
            while (total_read < args->msg_size)
            {
                ssize_t read_bytes = read(fds[0], msg, fifo_sz);

                if (read_bytes == -1)
                {
                    sys_error("Error reading from pipe");
                }

                total_read += read_bytes;
            }

            notify_server();
        }
    }

    close(fds[0]);
    free(msg);
}

void parent_flow(int fds[2], bench_args *args)
{
    close(fds[0]);

    struct sigaction sig_action;
    setup_server_signals(&sig_action);

    void *msg = malloc(args->msg_size);
    const int fifo_sz = fcntl(fds[1], F_GETPIPE_SZ);
    struct bench_results results;
    init_benchmark(&results);

    wait_for_signal(&sig_action);

    if (args->msg_count == 0)
    {
        bench_rw_results results;
        notify_client();
        results.start = now_us();
        ssize_t total_write = 0;

        while (total_write < args->msg_size)
        {
            ssize_t write_bytes = write(fds[1], msg, (args->msg_size - total_write > fifo_sz) ? fifo_sz : args->msg_size - total_write);

            if (write_bytes == -1)
            {
                sys_error("Error writing buffer");
            }
            total_write += write_bytes;
        }

        results.end = now_us();
        FILE *fp = fopen(PIPE_SERVER_OUT, "w");
        evaluate_rw_benchmark(&results, args, fp);
        fclose(fp);
    }
    else
    {
        for (int msg_count = 0; msg_count < args->msg_count; msg_count++)
        {
            notify_client();
            results.iteration_start = now();
            ssize_t total_write = 0;
            while (total_write < args->msg_size)
            {
                ssize_t write_bytes = write(fds[1], msg, (args->msg_size - total_write > fifo_sz) ? fifo_sz : args->msg_size - total_write);

                if (write_bytes == -1)
                {
                    sys_error("Error writing to pipe");
                }
                total_write += write_bytes;
            }

            wait_for_signal(&sig_action);
            benchmark(&results);
        }

        evaluate_benchmark(&results, args, NULL);
    }
    close(fds[1]);
    free(msg);
}

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    int fds[2];

    if (pipe(fds) < 0)
    {
        sys_error("Error opening pipe");
    }

    pid_t pid;

    if ((pid = fork()) == -1)
    {
        sys_error("Error forking process");
    }

    if (pid == 0)
    {
        child_flow(fds, &args);
    }
    else
    {
        parent_flow(fds, &args);
    }

    return 0;
}