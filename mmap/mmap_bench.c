#include "common/common.h"
#include "mmap_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    exec_server_client(MMAP_SERVER, MMAP_CLIENT, &args);
    return 0;
}