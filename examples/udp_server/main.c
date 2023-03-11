#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <lib_sock.h>
#include <lib_poll.h>
#include <lib_log.h>

static int echo_server(int fd) {
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[1500];

    for(;;) {
        int ret;
        ret = poll(fd, POLLIN, 1000);
        if(ret < 0) {
            LIB_LOG_ERR("poll: %s", strerror(errno));
            return -1;
        }

        if(ret == 0)
            continue;

        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(fd, buf, sizeof(buf), 0,
                         (struct sockaddr *)&peer_addr, &peer_addr_len);
        if(nread == -1) {
            LIB_LOG_ERR("recvfrom: %s", strerror(errno));
            return -1;
        }

        if(nread == 0)
            continue;

        ret = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len,
                          host, NI_MAXHOST,
                          service, NI_MAXSERV,
                          NI_NUMERICSERV);

        if(ret == 0) {
            LIB_LOG_INFO("Received %zd bytes from %s:%s", nread, host, service);
        }
        else {
            LIB_LOG_ERR("getnameinfo: %s\n", gai_strerror(ret));
        }

        if(sendto(fd, buf, nread, 0,
                  (struct sockaddr *)&peer_addr,
                  peer_addr_len) != nread)
            fprintf(stderr, "Error sending response\n");
    }

    return 0;
}

int main(int argc, char **argv) {
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
        .ai_socktype = SOCK_DGRAM, /* Datagram socket */
        .ai_flags = AI_PASSIVE,    /* For wildcard IP address */
        .ai_protocol = 0,          /* Any protocol */
    };
    struct addrinfo *res, *rp;
	int ret = 0, fd = 0;
    char *service = NULL;
    __be16 port;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        //goto out;
    }
    else
        service = argv[1];

    /* getaddrinfo() returns a list of address structures.
        * Try each address until we successfully bind(2). If socket(2)
        * (or bind(2)) fails, we (close the socket and) try the next address.
        */
    ret = getaddrinfo("0.0.0.0", service, &hints, &res);
    if(ret != 0) {
        LIB_LOG_ERR("getaddrinfo (%d): %s", ret, gai_strerror(ret));
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

    if(!service) {
        if(lib_get_port(fd, &port) < 0) {
            LIB_LOG_ERR("lib_get_port");
        }
        else
            LIB_LOG_INFO("port: %d", ntohs(port));
    }

    echo_server(fd);

    close(fd);
out:
    return ret;
}