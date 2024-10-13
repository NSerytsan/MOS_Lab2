#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#include "common/common.h"
#include "shm_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    key_t seg_key = ftok(SHM_BENCH_PATH, 64);

    int seg_id = shmget(seg_key, args.msg_size + 1, IPC_CREAT | 0666);

    if (seg_id < 0)
    {
        sys_error("Couldn't get segment");
    }

    char *shared_mem = (char *)shmat(seg_id, NULL, 0);

    void *msg = malloc(args.msg_size);

    atomic_char *guard = (atomic_char *)shared_mem; // Use the first byte of the shared memory as guard for communication with server
    atomic_init(guard, 's');
    char *msg_shared_mem = shared_mem + 1;

    for (int msg_count = args.msg_count; msg_count > 0; msg_count--)
    {
        while (atomic_load(guard) != 'c')
            ;
        memcpy(msg, msg_shared_mem, args.msg_size);
        memset(msg_shared_mem, '#', args.msg_size);

        atomic_store(guard, 's');
    }

    free(msg);
    shmdt(shared_mem);

    return 0;
}