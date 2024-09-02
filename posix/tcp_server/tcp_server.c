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
#include <pthread.h>

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
        fprintf(stderr, "getaddrinfo (%d): %s\n", ret, gai_strerror(ret));
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

struct peer_data_s {
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    int sd;
};

static void *echo_server(void *arg) {
    struct peer_data_s *pd = (struct peer_data_s *)arg;
    char node[NI_MAXHOST], service[NI_MAXSERV];
    ssize_t nread;
    char buf[1500];
    int ret;

    ret = getnameinfo((struct sockaddr *)&pd->peer_addr,
                      pd->peer_addr_len,
                      node, sizeof(node),
                      service, sizeof(service),
                      NI_NUMERICSERV);
    if(ret < 0) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(ret));
    }

    for(;;) {
        ret = pollin(pd->sd, 1000);
        if(ret < 0) {
            fprintf(stderr, "poll: %s\n", strerror(errno));
            goto err;
        }

        if(ret == 0) {
            continue;
        }

        nread = recv(pd->sd, buf, sizeof(buf), 0);
        if(nread == -1) {
            fprintf(stderr, "recvfrom: %s\n", strerror(errno));
            goto err;
        }

        if(nread == 0) {
            continue;
        }

        fprintf(stdout, "received %zd bytes from %s:%s\n", nread, node, service);
        fflush(stdout);

        if(send(pd->sd, buf, nread, 0) != nread) {
            fprintf(stderr, "sendto: %s\n", strerror(errno));
        }
    }
err:
    pthread_exit(NULL);
}

static int tcp_server(int sd) {
    char node[NI_MAXHOST], service[NI_MAXSERV];
    struct peer_data_s *pd;
    pthread_t thread_id;

    for(;;) {
        int ret;

        pd = calloc(1, sizeof(*pd));
        if(!pd) {
            return -1;
        }

        ret = listen(sd, 1);
        if(ret < 0) {
            fprintf(stderr, "listen: %s\n", strerror(errno));
            free(pd);
            return -1;
        }

        pd->peer_addr_len = sizeof(pd->peer_addr);
        pd->sd = accept(sd, (struct sockaddr *)&pd->peer_addr,
                        &pd->peer_addr_len);
        if(pd->sd < 0) {
            fprintf(stderr, "accept: %s\n", strerror(errno));
            free(pd);
            return -1;
        }

        ret = getnameinfo((struct sockaddr *)&pd->peer_addr,
                          pd->peer_addr_len,
                          node, NI_MAXHOST,
                          service, NI_MAXSERV,
                          NI_NUMERICSERV);

        if(ret == 0) {
            fprintf(stderr, "accepted %s:%s\n", node, service);
        }
        else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(ret));
        }

        pthread_create(&thread_id, NULL, echo_server, pd);
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

    sd = socket_tcp("0.0.0.0", service);
    if(sd < 0) {
        fprintf(stderr, "failed to get socket: %s\n", strerror(errno));
        return -1;
    }

    if(socket_port(sd, &port) < 0) {
        fprintf(stderr, "failed to get port: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    fprintf(stdout, "listening tcp on 0.0.0.0:%d\n", ntohs(port));
    tcp_server(sd);
    close(sd);

    return 0;
}