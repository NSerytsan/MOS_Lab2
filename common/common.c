#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"

#define DEFAULT_MSG_COUNT 0
#define DEFAULT_MSG_SIZE 1024*1024

#define IGNORE_USR1 0x0
#define IGNORE_USR2 0x0
#define BLOCK_USR1 0x1
#define BLOCK_USR2 0x2

// Input args routines
void usage(const char *prog_name)
{
    fprintf(stderr, "Usage: <name of ipc>_bench -s <message size> -c <message count>\n");
    exit(EXIT_FAILURE);
}

void get_bench_args(bench_args *args, int argc, char **argv)
{
    if (args == NULL)
        return;
    int opt = 0;

    args->msg_count = DEFAULT_MSG_COUNT;
    args->msg_size = DEFAULT_MSG_SIZE;
    while ((opt = getopt(argc, argv, "c:s:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            args->msg_count = atoi(optarg);
            break;
        case 's':
            args->msg_size = atoi(optarg);
            break;
        default:
            continue;
        }
    }
}

// Error routines
void sys_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// Benchmark routines
unsigned long long now()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

double now_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) * 1000000.0 + (double)(tv.tv_usec);
}

void init_benchmark(bench_results *bench)
{
    if (bench == NULL)
        return;
    bench->min_time = INT32_MAX;
    bench->max_time = 0;
    bench->sum = 0;
    bench->squared_sum = 0;
    bench->total_start = now();
}

void benchmark(bench_results *bench)
{
    const unsigned long long time = now() - bench->iteration_start;
    if (time < bench->min_time)
    {
        bench->min_time = time;
    }

    if (time > bench->max_time)
    {
        bench->max_time = time;
    }

    bench->sum += time;
    bench->squared_sum += (time * time);
}

void evaluate_benchmark(bench_results *bench, bench_args *args, FILE *fp)
{
    if (fp == NULL)
        fp = stdout;

    size_t msg_count = (args->msg_count == 0) ? 1 : args->msg_count;
    const unsigned long total_time = now() - bench->total_start;
    const double average = ((double)bench->sum) / msg_count;

    double sigma = bench->squared_sum / msg_count;
    sigma = sqrt(sigma - (average * average));

    int message_rate = (int)(msg_count / (total_time / 1e9));

    fprintf(fp, "\nMessage size:       %lu\n", args->msg_size);
    fprintf(fp, "Message count:      %lu\n", msg_count);
    fprintf(fp, "Total duration:     %.3f\tms\n", total_time / 1e6);
    fprintf(fp, "Average duration:   %.3f\tus\n", average / 1000.0);
    fprintf(fp, "Minimum duration:   %.3f\tus\n", bench->min_time / 1000.0);
    fprintf(fp, "Maximum duration:   %.3f\tus\n", bench->max_time / 1000.0);
    fprintf(fp, "Standard deviation: %.3f\tus\n", sigma / 1000.0);
    fprintf(fp, "Message rate:       %d\tmsg/s\n", message_rate);
}

void evaluate_rw_benchmark(bench_rw_results *bench, bench_args *args, FILE *fp)
{
    if (fp == NULL)
    {
        fp = stdout;
    }
    const double latency = (bench->end - bench->start) / 1000;
    const double throughput = (args->msg_size / (1024.0 * 1024.0)) / ((bench->end - bench->start) / 1000000.0);
    fprintf(fp, "\nMessage size:  %lu\n", args->msg_size);
    fprintf(fp, "Latency:       %.3f\tms\n", latency);
    fprintf(fp, "Throughput:    %.3f\tMB/s\n", throughput);
}

// Signals routines
void sig_handler(int signal)
{
}

void setup_ignored_signals(struct sigaction *signal_action, int flags)
{
    signal_action->sa_handler = sig_handler;

    if (!(flags & BLOCK_USR1))
    {
        if (sigaction(SIGUSR1, signal_action, NULL))
        {
            sys_error("Error registering SIGUSR1 signal handler for server");
        }
    }

    if (!(flags & BLOCK_USR2))
    {
        if (sigaction(SIGUSR2, signal_action, NULL))
        {
            sys_error("Error registering SIGUSR2 signal handler for client");
        }
    }
}

void setup_blocked_signals(struct sigaction *signal_action, int flags)
{
    signal_action->sa_handler = SIG_DFL;

    if (flags & BLOCK_USR1)
    {
        sigaddset(&signal_action->sa_mask, SIGUSR1);
    }

    if (flags & BLOCK_USR2)
    {
        sigaddset(&signal_action->sa_mask, SIGUSR2);
    }

    sigprocmask(SIG_BLOCK, &signal_action->sa_mask, NULL);
}

void setup_signals(struct sigaction *signal_action, int flags)
{
    signal_action->sa_flags = SA_RESTART;

    sigemptyset(&signal_action->sa_mask);
    setup_ignored_signals(signal_action, flags);

    sigemptyset(&signal_action->sa_mask);
    setup_blocked_signals(signal_action, flags);
}

void setup_client_signals(struct sigaction *signal_action)
{
    setup_signals(signal_action, IGNORE_USR1 | BLOCK_USR2);
    usleep(1000);
}

void setup_server_signals(struct sigaction *signal_action)
{
    setup_signals(signal_action, BLOCK_USR1 | IGNORE_USR2);
    usleep(1000);
}

void wait_for_signal(struct sigaction *signal_action)
{
    int signal_number;
    sigwait(&(signal_action->sa_mask), &signal_number);
}

void notify_server()
{
    kill(0, SIGUSR1);
}

void notify_client()
{
    kill(0, SIGUSR2);
}

void setup_parent_signals()
{
    struct sigaction sig_action;
    setup_signals(&sig_action, IGNORE_USR1 | IGNORE_USR2);
}

// Process routines
pid_t exec_process(char *path, bench_args *args)
{
    const static size_t buff_sz = 256;
    char opt_c_buffer[buff_sz];
    snprintf(opt_c_buffer, buff_sz, "-c %ld", args->msg_count);
    char opt_s_buffer[buff_sz];
    snprintf(opt_s_buffer, buff_sz, "-s %ld", args->msg_size);

    char *argv[4] = {path};
    argv[1] = opt_c_buffer;
    argv[2] = opt_s_buffer;
    argv[3] = NULL;

    const pid_t parent_pid = getpid();
    const pid_t pid = fork();

    if (pid == 0)
    {
        setpgid(pid, parent_pid);

        if (execv(argv[0], argv) == -1)
        {
            sys_error("Error opening child process");
        }
    }

    return pid;
}

void exec_server_client(char *server_path, char *client_path, bench_args *args)
{
    setup_parent_signals();

    pid_t server_pid = exec_process(server_path, args);
    pid_t client_pid = exec_process(client_path, args);

    waitpid(server_pid, NULL, WUNTRACED);
    waitpid(client_pid, NULL, WUNTRACED);
}