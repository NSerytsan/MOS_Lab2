#include "common/common.h"
#include "fifo.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    exec_server_client(FIFO_SERVER, FIFO_CLIENT, &args);

    return 0;
}