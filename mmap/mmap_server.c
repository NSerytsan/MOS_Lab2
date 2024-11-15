#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "mmap_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);


    char *addr = mmap(NULL, args.msg_size, PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
    if (addr == MAP_FAILED)
    {
        sys_error("Error mapping file.");
    }

    // Communication with client
    void *msg = malloc(args.msg_size);
    atomic_char *guard = (atomic_char *)addr;

    bench_results results;
    init_benchmark(&results);

    while (atomic_load(guard) != 's')
        ;

    if (args.msg_count == 0)
    {
        bench_rw_results results;
        results.start = now_us();
        memset(addr, '#', args.msg_size);
        results.end = now_us();
        atomic_store(guard, 'c');
        FILE *fp = fopen(MMAP_SERVER_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        for (int msg_count = 0; msg_count < args.msg_count; ++msg_count)
        {
            results.iteration_start = now();

            memset(addr, '#', args.msg_size);

            atomic_store(guard, 'c');
            while (atomic_load(guard) != 's')
                ;

            memcpy(msg, addr, args.msg_size);

            benchmark(&results);
        }

        evaluate_benchmark(&results, &args, NULL);
    }

    free(msg);

    if (munmap(addr, args.msg_size) < 0)
    {
        sys_error("Error unmapping file.");
    }

    return 0;
}