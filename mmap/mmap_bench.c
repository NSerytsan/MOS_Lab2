#include <sys/mman.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "mmap_common.h"

void client_flow(char *addr, bench_args *args)
{
    // Communication with sever
    void *msg = malloc(args->msg_size);

    atomic_char *guard = (atomic_char *)addr;

    atomic_store(guard, 's');

    if (args->msg_count == 0)
    {
        bench_rw_results results;
        while (atomic_load(guard) != 'c')
            ;
        results.start = now_us();
        memcpy(msg, addr + 1, args->msg_size);
        results.end = now_us();
        FILE *fp = fopen(MMAP_CLIENT_OUT, "w");
        evaluate_rw_benchmark(&results, args, fp);
        fclose(fp);
    }
    else
    {
        size_t msg_count = args->msg_count;
        for (; args->msg_count > 0; --msg_count)
        {
            while (atomic_load(guard) != 'c')
                ;

            memcpy(msg, addr + 1, args->msg_size);
            memset(addr + 1, '#', args->msg_size);

            atomic_store(guard, 's');
        }
    }

    free(msg);
}

void server_flow(char *addr, bench_args *args)
{
    // Communication with client
    void *msg = malloc(args->msg_size);
    atomic_char *guard = (atomic_char *)addr;

    bench_results results;
    init_benchmark(&results);

    while (atomic_load(guard) != 's')
        ;

    if (args->msg_count == 0)
    {
        bench_rw_results results;
        results.start = now_us();
        memset(addr + 1, '#', args->msg_size);
        results.end = now_us();
        atomic_store(guard, 'c');
        FILE *fp = fopen(MMAP_SERVER_OUT, "w");
        evaluate_rw_benchmark(&results, args, fp);
        fclose(fp);
    }
    else
    {
        for (int msg_count = 0; msg_count < args->msg_count; ++msg_count)
        {
            results.iteration_start = now();

            memset(addr + 1, '#', args->msg_size);

            atomic_store(guard, 'c');
            while (atomic_load(guard) != 's')
                ;

            memcpy(msg, addr + 1, args->msg_size);

            benchmark(&results);
        }

        evaluate_benchmark(&results, args, NULL);
    }

    free(msg);
}

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    char *addr = mmap(NULL, args.msg_size + 1, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
    {
        sys_error("Error mapping file.");
    }

    pid_t pid;

    if ((pid = fork()) == -1)
    {
        sys_error("Error forking process");
    }

    if (pid == 0)
    {
        client_flow(addr, &args);
    }
    else
    {
        server_flow(addr, &args);
    }

    if (munmap(addr, args.msg_size + 1) < 0)
    {
        sys_error("Error unmapping file.");
    }

    return 0;
}