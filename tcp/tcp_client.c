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
    struct addrinfo *server_info = NULL;
    if (getaddrinfo(TCP_BENCH_HOST, TCP_BENCH_PORT, &hints, &server_info) == -1)
    {
        sys_error("getaddrinfo failed");
    }

    struct addrinfo *addr_itr;
    int sock_fd = -1;
    for (addr_itr = server_info; addr_itr != NULL; addr_itr = addr_itr->ai_next)
    {
        if ((sock_fd = socket(addr_itr->ai_family, addr_itr->ai_socktype, addr_itr->ai_protocol)) == -1)
            continue;

        if (connect(sock_fd, addr_itr->ai_addr, addr_itr->ai_addrlen) == -1)
        {
            close(sock_fd);
            continue;
        }
        break;
    }
    if (addr_itr == NULL)
    {
        sys_error("Error finding valid address");
    }

    freeaddrinfo(server_info);

    void *msg = malloc(args.msg_size);

    if (args.msg_count == 0)
    {
        bench_rw_results results;
        results.start = now_us();
        ssize_t recv_bytes = recv(sock_fd, msg, args.msg_size, 0);
        ssize_t total_recv_bytes = recv_bytes;
        while (recv_bytes > 0 && total_recv_bytes < args.msg_size)
        {
            recv_bytes = recv(sock_fd, msg, args.msg_size, 0);
            if (recv_bytes > 0)
                total_recv_bytes += recv_bytes;
        }

        if (recv_bytes == -1)
        {
            sys_error("Error receiving data on client-side");
        }
        results.end = now_us();
        FILE *fp = fopen(TCP_CLIENT_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        for (int msg_count = args.msg_count; msg_count > 0; msg_count--)
        {
            ssize_t recv_bytes = recv(sock_fd, msg, args.msg_size, 0);
            ssize_t total_recv_bytes = recv_bytes;
            while (recv_bytes > 0 && total_recv_bytes < args.msg_size)
            {
                recv_bytes = recv(sock_fd, msg, args.msg_size, 0);
                if (recv_bytes > 0)
                    total_recv_bytes += recv_bytes;
            }

            if (recv_bytes == -1)
            {
                sys_error("Error receiving data on client-side");
            }

            memset(msg, '#', args.msg_size);

            if (send(sock_fd, msg, args.msg_size, 0) == -1)
            {
                sys_error("Error sending data on client-side");
            }
        }
    }

    close(sock_fd);
    free(msg);

    return 0;
}