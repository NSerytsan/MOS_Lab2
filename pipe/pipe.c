#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/common.h"

void child_flow(int fds[2], bench_args *args)
{
    close(fds[1]);

    FILE *fp = fdopen(fds[0], "r");
    if (fp == NULL)
    {
        sys_error("Couldn't open stream for reading");
    }

    struct sigaction sig_action;
    setup_client_signals(&sig_action);
    void *msg_buffer = malloc(args->msg_size);

    notify_server();

    for (; args->msg_count > 0; --args->msg_count)
    {
        wait_for_signal(&sig_action);

        if (fread(msg_buffer, args->msg_size, 1, fp) == -1)
        {
            sys_error("Error reading from pipe");
        }

        notify_server();
    }

    close(fds[1]);
    free(msg_buffer);
}

void parent_flow(int fds[2], bench_args *args)
{
    close(fds[0]);

    FILE *fp = fdopen(fds[1], "w");
    if (fp == NULL)
    {
        sys_error("Couldn't open stream for writting");
    }

    struct sigaction sig_action;
    setup_server_signals(&sig_action);

    void *msg_buffer = malloc(args->msg_size);

    struct bench_results results;
    init_benchmakr(&results);

    wait_for_signal(&sig_action);

    for (int msg = 0; msg < args->msg_count; msg++)
    {
        results.iteration_start = now();

        if (fwrite(msg_buffer, args->msg_size, 1, fp) == -1)
        {
            sys_error("Error writing to pipe");
        }

        fflush(fp);

        notify_client();
        wait_for_signal(&sig_action);
        benchmark(&results);
    }

    evaluate_benchmark(&results, args);

    close(fds[1]);
    free(msg_buffer);
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