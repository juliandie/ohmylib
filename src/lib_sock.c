#include <lib_sock.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

#include <sys/ioctl.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <linux/types.h>

#include <arpa/inet.h>

static int lib_setsockopt(int fd, int lvl, int optname, int optval) {
    return setsockopt(fd, lvl, optname, &optval, sizeof(int));
}

static int lib_getsockopt(int fd, int lvl, int optname) {
    int val;
    uint32_t len = sizeof(int);

    if(getsockopt(fd, lvl, optname, &val, &len))
        return -1;

    return val;
}

int lib_bind4(int fd, __be32 addr, __be16 port) {
    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port = port,
        .sin_addr.s_addr = addr,
    };

    return bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr));
}

int lib_bind6(int fd, struct in6_addr *addr, __be16 port) {
    struct sockaddr_in6 sa;

    sa.sin6_family = AF_INET6;
    sa.sin6_port = port;
    memcpy(sa.sin6_addr.s6_addr, addr, sizeof(struct in6_addr));

    return bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr));
}

int lib_get_port(int fd, __be16 *port) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in sa;
    int ret;

    ret = getsockname(fd, (struct sockaddr *)&sa, &addrlen);

    if(port)
        *port = sa.sin_port;

    return ret;
}

int lib_recv(int fd, void *buf, size_t len, int flags) {
    struct pollfd fds;
    void *p = buf;
    size_t count = 0;

    fds.fd = fd;
    fds.events = POLLIN;
    while(count < len) {
        int ret;
        ret = poll(&fds, 1, 1000);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        //ret = recvfrom(fd, p, len - count, flags, NULL, NULL);
        ret = recv(fd, p, len - count, flags);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        count += (size_t)ret;
        p = (char *)p + ret;
    }

    return count;
}

#if 0
static int lib_parse_msghdr(struct msghdr *msgh, int *sysidx, bool *broadcast) {
    struct in_pktinfo *pktinfo;
    struct cmsghdr *cmsg;

    if(msgh->msg_flags & MSG_CTRUNC) {
        return -1;
    }

    for(cmsg = CMSG_FIRSTHDR(msgh); cmsg != NULL; cmsg = CMSG_NXTHDR(msgh, cmsg)) {
        if(cmsg->cmsg_type == IP_PKTINFO) {
            pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);

            if(sysidx)
                *sysidx = pktinfo->ipi_ifindex;

            if(broadcast)
                *broadcast = (pktinfo->ipi_addr.s_addr == INADDR_BROADCAST);
        }
    }

    return 0;
}

int lib_recvmsg(int fd, void *buf, size_t len, int flags) {
    struct msghdr msgh = { 0 };
    struct iovec io = {
        .iov_base = buf,
        .iov_len = len
    };
    union {
        char buf[CMSG_SPACE(sizeof(struct in_pktinfo))];
        struct cmsghdr align;
    } u;
    bool broadcast;
    int sysidx;
    int ret;

    //msg.msg_name = &frame->addr;
    //msg.msg_namelen = sizeof(struct sockaddr_in);
    msgh.msg_iov = &io;
    msgh.msg_iovlen = 1;
    msgh.msg_control = u.buf;
    msgh.msg_controllen = sizeof(u.buf);

    ret = recvmsg(fd, &msgh, flags);
    if(ret < 0)
        return -1;

    // drop truncated packages
    if(msgh.msg_flags & MSG_TRUNC)
        return -1;

    if(parse_msghdr(msgh, &sysidx, &broadcast) < 0)
        return -1;

    if(lib_netif_has_adr(lib_netif_name(frame->sysidx), &frame->addr) &&
       frame->addr.sin_port == htons(udp->port)) {
        LIB_LOG_WARNING("broadcast recursion detected"); // sending broadcasts to myself
        goto err;
    }

err:
    return ret;
}
#endif

int lib_send(int fd, const void *buf, size_t len, int flags) {
    const void *p = buf;
    size_t count = 0;

    while(count < len) {
        int ret;
        ret = send(fd, p, len - count, flags);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        count += (size_t)ret;
        p = (char *)p + ret;
    }

    return count;
}

int lib_sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
    const void *p = buf;
    size_t count = 0;

    while(count < len) {
        int ret;
        ret = sendto(fd, p, len - count, flags, dest_addr, addrlen);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        count += (size_t)ret;
        p = (char *)p + ret;
    }

    return count;
}

int lib_flush(int fd, uint32_t udelay) {
    int queue;

    if(udelay == 0)
        udelay = 1;

    do {
        int ret;

        ret = ioctl(fd, TIOCOUTQ, &queue);
        if(ret != 0)
            return -1;

        if(queue)
            usleep(udelay);

    } while(queue);

    return 0;
}

int lib_sock_reuseaddr(int fd) {
    return lib_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 1);
}

/**
 * IP_TOS		    ( 1)
 * IP_TTL		    ( 2)
 * IP_HDRINCL	    ( 3)
 * IP_OPTIONS	    ( 4)
 * IP_ROUTER_ALERT	( 5)
 * IP_RECVOPTS	    ( 6)
 * IP_RETOPTS	    ( 7)
 * IP_PKTINFO	    ( 8)
 * IP_PKTOPTIONS	( 9)
 * IP_MTU_DISCOVER	(10)
 * IP_RECVERR	    (11)
 * IP_RECVTTL	    (12)
 *	IP_RECVTOS	    (13)
 * IP_MTU		    (14)
 * IP_FREEBIND	    (15)
 * IP_IPSEC_POLICY	(16)
 * IP_XFRM_POLICY	(17)
 * IP_PASSSEC	    (18)
 * IP_TRANSPARENT	(19)
 */

// see IP_MTU_DISCOVER values (linux/in.h)
int lib_sock_get_mtu_discover(int fd) {
    return lib_getsockopt(fd, SOL_IP, IP_MTU_DISCOVER);
}

int lib_sock_set_mtu_discover_want(int fd, int mtu_discover) {
    return lib_setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, mtu_discover);
}

int lib_sock_get_mtu(int fd) {
    return lib_getsockopt(fd, SOL_IP, IP_MTU);
}

int lib_sock_get_pktinfo(int fd, int enable) {
    return lib_setsockopt(fd, SOL_IP, IP_PKTINFO, enable ? 1 : 0);
}

/**
 * SO_DEBUG	        ( 1)
 * SO_REUSEADDR	    ( 2)
 * SO_TYPE		    ( 3)
 * SO_ERROR	        ( 4)
 * SO_DONTROUTE	    ( 5)
 * SO_BROADCAST	    ( 6)
 * SO_SNDBUF	    ( 7)
 * SO_RCVBUF	    ( 8)
 * SO_SNDBUFFORCE	(32)
 * SO_RCVBUFFORCE	(33)
 * SO_KEEPALIVE	    ( 9)
 * SO_OOBINLINE	    (10)
 * SO_NO_CHECK	    (11)
 * SO_PRIORITY	    (12)
 * SO_LINGER	    (13)
 * SO_BSDCOMPAT	    (14)
 * SO_REUSEPORT	    (15)
 */
int lib_net_get_broadcast(int fd, int enable) {
    return lib_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, enable ? 1 : 0);
}

int lib_sock_bind_to_if(int fd, const char *ifname) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), ifname);
    return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
}