#include <sys/shm.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#include "common/common.h"
#include "shm.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    key_t seg_key = ftok(SHM_BENCH_PATH, 64);

    int seg_id = shmget(seg_key, 1 + args.msg_size, IPC_CREAT | 0666);
    if (seg_id < 0)
    {
        sys_error("Error allocating segment");
    }

    char *shared_mem = (char *)shmat(seg_id, NULL, 0);
    if (shared_mem == (char *)-1)
    {
        sys_error("Error attaching segment");
    }

    void *msg = malloc(args.msg_size);
    atomic_char *guard = (atomic_char *)shared_mem; // Use the first byte of the shared memory as guard for communication with client

    while (atomic_load(guard) != 's') // Waiting the notification from client
        ;

    bench_results results;
    init_benchmark(&results);

    for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
    {
        results.iteration_start = now();

        memset(shared_mem + 1, '#', args.msg_size); // Write the message into shared memory

        atomic_store(guard, 'c');         // Notifying the client that the message can be read from shared memory
        while (atomic_load(guard) != 's') // Waiting the notification from client
            ;

        memcpy(msg, shared_mem + 1, args.msg_size); // Read message from shared memory

        benchmark(&results);
    }

    evaluate_benchmark(&results, &args);

    free(msg);
    shmdt(shared_mem);
    shmctl(seg_id, IPC_RMID, NULL);

    return 0;
}