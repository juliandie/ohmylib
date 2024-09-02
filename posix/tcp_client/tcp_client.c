#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <linux/ethtool.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int pollin(int fd, int timeout) {
    struct pollfd fds = {
        .fd = fd,
        .events = POLLIN,
    };
    return poll(&fds, 1, timeout);
}

static int socket_tcp(const char *node, const char *service) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC, /* Allow IPv4 or IPv6 */
        .ai_socktype = SOCK_STREAM, /* TCP socket */
        .ai_flags = AI_PASSIVE,
        .ai_protocol = 0,
    };
    struct addrinfo *res;
    int ret, sd = -1;

    ret = getaddrinfo(node, service, &hints, &res);
    if(ret != 0) {
        fprintf(stderr, "getaddrinfo (%d): %s", ret, gai_strerror(ret));
        return -1;
    }

    for(struct addrinfo *rp = res; rp != NULL; rp = rp->ai_next) {
        sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sd == -1)
            continue;

        if(bind(sd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        close(sd);
    }

    freeaddrinfo(res);
    return sd;
}

static int client(int sd, int argc, char **argv) {
    char buf[1500];

    for(int j = 3; j < argc; j++) {
        int ret;
        ssize_t nread, len;
        len = strlen(argv[j]);

        if(len > (ssize_t)sizeof(buf)) {
            fprintf(stderr, "Ignoring long message in argument %d\n", j);
            continue;
        }

        if(send(sd, argv[j], len, 0) != len) {
            fprintf(stderr, "send: %s\n", strerror(errno));
            return -1;
        }

        ret = pollin(sd, 1000);
        if(ret < 0) {
            fprintf(stderr, "poll: %s\n", strerror(errno));
            return -1;
        }

        if(ret == 0) {
            continue;
        }

        nread = recv(sd, buf, sizeof(buf), 0);
        if(nread == -1) {
            fprintf(stderr, "recv: %s\n", strerror(errno));
            return -1;
        }

        if(nread == 0) {
            continue;
        }

        fprintf(stdout, "received %zd bytes\n", nread);
        fflush(stdout);
    }

    return 0;
}

int main(int argc, char **argv) {
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    int sd = 0;
    int ret;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port> <msg>\n", argv[0]);
        return -1;
    }

    sd = socket_tcp("0.0.0.0", NULL);
    if(sd < 0) {
        fprintf(stderr, "failed to get socket: %s\n", strerror(errno));
        return -1;
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &peer_addr.sin_addr);
    peer_addr_len = sizeof(peer_addr);
    ret = connect(sd, (struct sockaddr *)&peer_addr, peer_addr_len);
    if(ret < 0) {
        fprintf(stderr, "failed to connect: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    client(sd, argc, argv);
    close(sd);
    return 0;
}