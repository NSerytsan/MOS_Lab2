#include "common/common.h"
#include "tcp_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    exec_server_client(TCP_SERVER, TCP_CLIENT, &args);

    return 0;
}