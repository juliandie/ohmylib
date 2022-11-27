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

int lib_port(int fd, __be16 *port) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in sa;
    int ret;

    ret = getsockname(fd, (struct sockaddr *)&sa, &addrlen);

    if(port)
        *port = sa.sin_port;

    return ret;
}

int lib_select(int fd, int timeout) {
    struct timeval tv;
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    /* Timeout. */
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(fd + 1, &fds, NULL, NULL, &tv);
}

int lib_poll(int fd, int timeout) {
    struct pollfd fds;

    fds.fd = fd;
    fds.events = POLLIN;
    return poll(&fds, 1, timeout);
}

int lib_recv(int fd, void *buf, size_t len, int flags) {
    void *p = buf;
    size_t count = 0;
    int ret = 0;

    while(count < len) {
        ret = lib_poll(fd, 1000);
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
        p += ret;
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
    int ret = 0;

    while(count < len) {
        ret = send(fd, p, len - count, flags);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        count += (size_t)ret;
        p += ret;
    }

    return count;
}

int lib_sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
    const void *p = buf;
    size_t count = 0;
    int ret = 0;

    while(count < len) {
        ret = sendto(fd, p, len - count, flags, dest_addr, addrlen);
        if(ret < 0)
            return -1;

        if(ret == 0)
            return count;

        count += (size_t)ret;
        p += ret;
    }

    return count;
}

int lib_flush(int fd, uint32_t udelay) {
    int ret, queue;

    if(udelay == 0)
        udelay = 1;

    do {
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

int lib_sock_mtu_discover(int fd) {
    return lib_getsockopt(fd, SOL_IP, IP_MTU_DISCOVER);
}

int lib_sock_mtu_discover_want(int fd) {
    return lib_setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_WANT);
}

int lib_sock_mtu_discover_dont(int fd) { // do fragment?
    return lib_setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_DONT);
}

int lib_sock_mtu_discover_do(int fd) { // don't fragment?
    return lib_setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_DO);
}

int lib_sock_mtu_discover_probe(int fd) {
    return lib_setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_PROBE);
}

int lib_sock_mtu(int fd) {
    return lib_getsockopt(fd, SOL_IP, IP_MTU);
}

int lib_sock_pktinfo(int fd, bool enable) {
    if(enable)
        return lib_setsockopt(fd, SOL_IP, IP_PKTINFO, 1);
    else
        return lib_setsockopt(fd, SOL_IP, IP_PKTINFO, 0);
}

int lib_net_broadcast(int fd, bool enable) {
    if(enable)
        return lib_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, 1);
    else
        return lib_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, 0);
}

int lib_sock_bind_to_if(int fd, const char *ifname) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), ifname);
    return setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
}