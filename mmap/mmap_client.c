#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "mmap.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    int fd = open(MMAP_BENCH_FILE, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        sys_error("Error opening file");
    }

    char *addr = mmap(NULL, args.msg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        sys_error("Error mapping file");
    }

    // Comunication with sever
    void *msg_buffer = malloc(args.msg_size);
    
    atomic_char *guard = (atomic_char *)addr;

    atomic_store(guard, 's');
    for (; args.msg_count > 0; --args.msg_count)
    {
        while (atomic_load(guard) != 'c')
            ;

        memcpy(msg_buffer, addr, args.msg_size);
        memset(addr, '#', args.msg_size);

        atomic_store(guard, 's');
    }

    free(msg_buffer);

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