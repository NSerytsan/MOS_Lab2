#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "common/common.h"
#include "unix_domain_common.h"

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

    int conn_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_sz);
    if (conn_fd == -1)
    {
        sys_error("Error accepting connection");
    }

    void *msg = malloc(args.msg_size);
    if (args.msg_count == 0)
    {
        bench_rw_results results;
        results.start = now_us();

        if (send(conn_fd, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error sending to server");
        }
        results.end = now_us();
        FILE *fp = fopen(UNIX_DOMAIN_SERVER_OUT, "w");
        evaluate_rw_benchmark(&results, &args, fp);
        fclose(fp);
    }
    else
    {
        bench_results results;
        init_benchmark(&results);

        for (int msg_count = 0; msg_count < args.msg_count; msg_count++)
        {
            results.iteration_start = now();
            if (send(conn_fd, msg, args.msg_size, 0) < args.msg_size)
            {
                sys_error("Error sending on server-side");
            }

            memset(msg, '#', args.msg_size);

            ssize_t total_recv = 0;
            while (total_recv < args.msg_size)
            {
                ssize_t recv_bytes = recv(conn_fd, msg, args.msg_size, 0);
                if (recv_bytes == -1)
                {
                    sys_error("Error receiving data on client-side");
                }

                total_recv += recv_bytes;
            }

            benchmark(&results);
        }

        evaluate_benchmark(&results, &args, NULL);
    }

    free(msg);
    close(sock_fd);
    close(conn_fd);

    return 0;
}