#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "tcp_common.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *srv_info = NULL;
    if (getaddrinfo(TCP_BENCH_HOST, TCP_BENCH_PORT, &hints, &srv_info) == -1)
    {
        sys_error("getaddrinfo failed");
    }

    int sock_fd = -1;
    struct addrinfo *valid_addr;
    for (valid_addr = srv_info; valid_addr != NULL; valid_addr = valid_addr->ai_next)
    {
        if ((sock_fd = socket(valid_addr->ai_family, valid_addr->ai_socktype, valid_addr->ai_protocol)) == -1)
        {
            continue;
        }

        if (bind(sock_fd, valid_addr->ai_addr, valid_addr->ai_addrlen) == -1)
        {
            close(sock_fd);
        }

        break;
    }

    if (valid_addr == NULL)
    {
        sys_error("Error finding valid address");
    }

    freeaddrinfo(srv_info);

    if (listen(sock_fd, 16) == -1)
    {
        sys_error("Error listening on given socket!");
    }

    struct sockaddr_storage addr;
    socklen_t sin_sz = sizeof addr;

    int conn_fd = accept(sock_fd, (struct sockaddr *)&addr, &sin_sz);
    if (conn_fd == -1)
    {
        sys_error("Error accepting");
    }

    void *msg = malloc(args.msg_size);
    bench_results results;
    init_benchmark(&results);
    for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
    {
        results.iteration_start = now();

        if (send(conn_fd, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error sending to server");
        }

        if (recv(conn_fd, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error receiving from server");
        }

        benchmark(&results);
    }

    evaluate_benchmark(&results, &args, NULL);

    close(sock_fd);
    close(conn_fd);
    free(msg);

    return 0;
}