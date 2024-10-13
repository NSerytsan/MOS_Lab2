#include "common/common.h"
#include "unix_domain.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    exec_server_client(UNIX_DOMAIN_SERVER, UNIX_DOMAIN_CLIENT, &args);
    return 0;
}