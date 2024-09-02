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

static int pollin(int fd, int timeout) {
    struct pollfd fds = {
        .fd = fd,
        .events = POLLIN,
    };
    return poll(&fds, 1, timeout);
}

static int socket_udp(const char *node, const char *service) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC, /* Allow IPv4 or IPv6 */
        .ai_socktype = SOCK_DGRAM, /* UDP socket */
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

static int socket_port(int fd, __be16 *port) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in sa;
    int ret;

    ret = getsockname(fd, (struct sockaddr *)&sa, &addrlen);

    if(port)
        *port = sa.sin_port;

    return ret;
}

static int echo_server(int sd) {
    char node[NI_MAXHOST], service[NI_MAXSERV];
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[1500];

    for(;;) {
        int ret;

        ret = pollin(sd, 1000);
        if(ret < 0) {
            fprintf(stderr, "poll: %s\n", strerror(errno));
            return -1;
        }

        if(ret == 0) {
            continue;
        }

        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(sd, buf, sizeof(buf), 0,
                         (struct sockaddr *)&peer_addr,
                         &peer_addr_len);
        if(nread == -1) {
            fprintf(stderr, "recvfrom: %s\n", strerror(errno));
            return -1;
        }

        if(nread == 0) {
            continue;
        }

        ret = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len,
                          node, sizeof(node),
                          service, sizeof(service),
                          NI_NUMERICSERV);
        if(ret == 0) {
            fprintf(stdout, "received %zd bytes from %s:%s\n", nread, node, service);
            fflush(stdout);
        }
        else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(ret));
        }

        if(sendto(sd, buf, nread, 0,
           (struct sockaddr *)&peer_addr,
           peer_addr_len) != nread)
            fprintf(stderr, "sendto: %s\n", strerror(errno));
    }

    return 0;
}

int main(int argc, char **argv) {
    char *service = NULL;
    __be16 port;
    int sd;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
    }
    else
        service = argv[1];

    sd = socket_udp("0.0.0.0", service);
    if(sd < 0) {
        fprintf(stderr, "failed to get socket: %s\n", strerror(errno));
        return -1;
    }

    if(socket_port(sd, &port) < 0) {
        fprintf(stderr, "failed to get port: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    fprintf(stdout, "listening udp on 0.0.0.0:%d\n", ntohs(port));
    echo_server(sd);
    close(sd);

    return 0;
}