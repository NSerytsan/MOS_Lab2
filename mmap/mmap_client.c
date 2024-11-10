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

    int fd = shm_open(MMAP_BENCH_FILE, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        sys_error("Error opening file");
    }

    char *addr = mmap(NULL, args.msg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        sys_error("Error mapping file");
    }

    // Communication with sever
    void *msg = malloc(args.msg_size);

    atomic_char *guard = (atomic_char *)addr;

    atomic_store(guard, 's');

    if (args.msg_count == 0)
    {
        bench_rw_results results;
        while (atomic_load(guard) != 'c')
            ;
        results.start = now_us();
        memcpy(msg, addr, args.msg_size);
        results.end = now_us();
        FILE *fp = fopen(MMAP_CLIENT_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        for (; args.msg_count > 0; --args.msg_count)
        {
            while (atomic_load(guard) != 'c')
                ;

            memcpy(msg, addr, args.msg_size);
            memset(addr, '#', args.msg_size);

            atomic_store(guard, 's');
        }
    }

    free(msg);

    if (munmap(addr, args.msg_size) < 0)
    {
        sys_error("Error unmapping file");
    }

    if (close(fd) < 0)
    {
        sys_error("Error closing file");
    }

    return 0;
}