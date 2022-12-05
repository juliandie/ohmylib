
#ifndef UDP_SOCK_H_
#define UDP_SOCK_H_

#include <linux/types.h> // __be16, __be32
#include <stdint.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>

#if 0
typedef uint32_t in_addr_t;

// sys/socket.h
struct sockaddr_in {
    __kernel_sa_family_t sin_family; /* Address family */
    __be16 sin_port; /* Port number */

    struct in_addr {
        in_addr_t s_addr;
    } sin_addr; /* Internet address */
};

struct sockaddr {
    sa_family_t	sa_family;	/* address family, AF_xxx	*/
    char		sa_data[14];	/* 14 bytes of protocol address	*/
};

struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};
#endif

/**
 * Create your socket yourself
 
	int fd;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		goto err;
 */

int lib_bind4(int fd, __be32 addr, __be16 port);
int lib_bind6(int fd, struct in6_addr *addr, __be16 port);

int lib_port(int fd, __be16 *port);

int lib_recv(int fd, void *buf, size_t len, int flags);
int lib_send(int fd, const void *buf, size_t len, int flags);
int lib_sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
int lib_flush(int fd, uint32_t udelay);

int lib_sock_reuseaddr(int fd);
int lib_sock_mtu_discover(int fd);
int lib_sock_mtu_discover_want(int fd);
int lib_sock_mtu_discover_dont(int fd);
int lib_sock_mtu_discover_do(int fd);
int lib_sock_mtu_discover_probe(int fd);
int lib_sock_mtu(int fd);
int lib_sock_pktinfo(int fd, bool enable);
int lib_net_broadcast(int fd, bool enable);
int lib_sock_bind_to_if(int fd, const char *ifname);

#endif
