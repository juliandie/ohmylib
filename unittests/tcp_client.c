#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <lib_sock.h>
#include <lib_log.h>

static volatile int run = 1;

static int client(int fd, int argc, char **argv) {
    char buf[1500];

    for(int j = 3; j < argc; j++) {
        ssize_t nread, len;
        len = strlen(argv[j]) + 1;

        if(len > (ssize_t)sizeof(buf)) {
            fprintf(stderr, "Ignoring long message in argument %d\n", j);
            continue;
        }

        if(write(fd, argv[j], len) != len) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }

        nread = read(fd, buf, sizeof(buf));
        if(nread == -1) {
            LIB_LOG_ERR("read: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("Received %zd bytes: %s\n", nread, buf);
    }
    return 0;
}

int main(int argc, char **argv) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,     /* Allow IPv4 or IPv6 */
        .ai_socktype = SOCK_STREAM, /* Datagram socket */
        .ai_flags = 0,
        .ai_protocol = 0,           /* Any protocol */
    };
    struct addrinfo *res, *rp;
    int ret = 0, fd = 0;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port> <msg>\n", argv[0]);
        goto out;
    }

    /* Obtain address(es) matching host/port */
    ret = getaddrinfo(argv[1], argv[2], &hints, &res);
    if(ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        goto out;
    }

    /* getaddrinfo() returns a list of address structures.
     * Try each address until we successfully bind(2). If socket(2)
     * (or bind(2)) fails, we (close the socket and) try the next address.
     */
    for(rp = res; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(fd == -1)
            continue;

        if(connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        close(fd);
    }

    freeaddrinfo(res);

    if(rp == NULL) { /* No address succeeded */
        LIB_LOG_ERR("bind: %s", strerror(errno));
        goto out;
    }

    client(fd, argc, argv);

    close(fd);
out:
    return ret;
}