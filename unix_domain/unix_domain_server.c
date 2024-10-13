#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "common/common.h"
#include "unix_domain.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        sys_error("Error opening socket on server-side");
    }

    remove(UNIX_DOMAIN_BENCH_FILE); // Remove the socket if it exists

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, UNIX_DOMAIN_BENCH_FILE);

    if (bind(sock_fd, (struct sockaddr *)&addr, SUN_LEN(&addr)) == -1)
    {
        sys_error("Error binding socket to address");
    }

    if (listen(sock_fd, 16) == -1)
    {
        sys_error("Couldn't start listening on socket");
    }

    struct sigaction sig_action;
    setup_server_signals(&sig_action);
    notify_client();

    struct sockaddr_un client_addr;
    socklen_t client_addr_sz = sizeof client_addr;

    int connection = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_sz);
    if (connection == -1)
    {
        sys_error("Error accepting connection");
    }

    void *msg = malloc(args.msg_size);
    bench_results results;
    init_benchmark(&results);

    for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
    {
        results.iteration_start = now();
        if (send(connection, msg, args.msg_size, 0) < args.msg_size)
        {
            sys_error("Error sending on server-side");
        }

        memset(msg, '#', args.msg_size);

        if (recv(connection, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error receiving on server-side");
        }

        benchmark(&results);
    }

    evaluate_benchmark(&results, &args);

    free(msg);
    close(sock_fd);
    close(connection);

    return 0;
}