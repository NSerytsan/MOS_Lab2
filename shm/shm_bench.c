#include "common/common.h"
#include "shm_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    exec_server_client(SHM_SERVER, SHM_CLIENT, &args);
    return 0;
}