#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include "common/common.h"
#include "unix_domain.h"

int main(int argc, char **argv)
{
    bench_args args;
    get_bench_args(&args, argc, argv);

    struct sigaction sig_action;
    setup_client_signals(&sig_action);
    wait_for_signal(&sig_action); // Waiting until the server setup socket

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        sys_error("Error opening socket on client-side");
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, UNIX_DOMAIN_BENCH_FILE);

    int connection = connect(sock_fd, (struct sockaddr *)&addr, SUN_LEN(&addr));
    if (connection == -1)
    {
        sys_error("Error connecting to server");
    }

    void *msg = malloc(args.msg_size);

    for (int msg_count = args.msg_count; msg_count > 0; msg_count--)
    {
        if (recv(connection, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error receiving on client-side");
        }

        memset(msg, '#', args.msg_size);

        if (send(connection, msg, args.msg_size, 0) == -1)
        {
            sys_error("Error sending on client-side");
        }
    }

    close(sock_fd);
    free(msg);

    return 0;
}