#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <lib_sock.h>
#include <lib_log.h>

static int echo_server(int fd) {
    ssize_t nread;
    char buf[1500];
    int ret;

    for(;;) {
        ret = lib_poll(fd, 1000);
        if(ret < 0) {
            LIB_LOG_ERR("poll: %s", strerror(errno));
            return -1;
        }

        if(ret == 0)
            continue;

        nread = read(fd, buf, sizeof(buf));
        if(nread == -1) {
            LIB_LOG_ERR("read: %s", strerror(errno));
            return -1;
        }

        if(nread == 0) {
            LIB_LOG_INFO("connection was closed");
            goto out;
        }

        if(write(fd, buf, nread) != nread)
            fprintf(stderr, "Error sending response\n");
    }
    out:
    return 0;
}

static int server(int fd) {
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    for(;;) {
        int ret, peer_fd;
        ret = listen(fd, 1); // accept only 1 conection at once
        if(ret < 0) {
            LIB_LOG_ERR("listen: %s", strerror(errno));
            continue;
        }

        peer_addr_len = sizeof(peer_addr);
        peer_fd = accept(fd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        if(peer_fd < 0) {
            LIB_LOG_ERR("accept: %s", strerror(errno));
            continue;
        }

        ret = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len,
                          host, NI_MAXHOST,
                          service, NI_MAXSERV,
                          NI_NUMERICSERV);

        if(ret == 0) {
            LIB_LOG_INFO("Accepted %s:%s", host, service);
        }
        else {
            LIB_LOG_ERR("getnameinfo: %s\n", gai_strerror(ret));
        }

        echo_server(peer_fd);
    }

    return 0;
}

int main(int argc, char **argv) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,     /* Allow IPv4 or IPv6 */
        .ai_socktype = SOCK_STREAM, /* TCP socket */
        .ai_flags = AI_PASSIVE,     /* For wildcard IP address */
        .ai_protocol = 0,           /* Any protocol */
    };
    struct addrinfo *res, *rp;
    int ret = 0, fd = 0;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        goto out;
    }

    /* getaddrinfo() returns a list of address structures.
     * Try each address until we successfully bind(2). If socket(2)
     * (or bind(2)) fails, we (close the socket and) try the next address.
     */
    ret = getaddrinfo(NULL, argv[1], &hints, &res);
    if(ret != 0) {
        LIB_LOG_ERR("getaddrinfo: %s", gai_strerror(ret));
        goto out;
    }

    for(rp = res; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(fd == -1)
            continue;

        if(bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        close(fd);
    }

    freeaddrinfo(res);

    if(rp == NULL) { /* No address succeeded */
        LIB_LOG_ERR("bind: %s", strerror(errno));
        goto out;
    }

    server(fd);

    close(fd);
out:
    return ret;
}