/**
 * @file    sys_udp.c
 *
 * Linux specific UDP implementation
 */
#include <lib_udp.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <linux/types.h>

#include <arpa/inet.h>

#include <lib_log.h>
#include <lib_buffer.h>
#include <lib_netif.h>

void udp_dump_rx_frame(lib_netpkt* frame) {
    LIB_LOG_DEBUG("Received %zuB from %s:%d %s\n",
            frame->rx.len, inet_ntoa(frame->addr.sin_addr),
            ntohs(frame->addr.sin_port), frame->broadcast ? "[BROADCAST]" : "");
    lib_dump(frame->rx.data, frame->rx.len, "RX Data [%dB]", frame->rx.len);
}

void udp_dump_tx_frame(lib_netpkt* frame) {
    LIB_LOG_DEBUG("Transmit %zuB to %s:%d %s\n",
            frame->tx.len, inet_ntoa(frame->addr.sin_addr),
            ntohs(frame->addr.sin_port), frame->broadcast ? "[BROADCAST]" : "");
    lib_dump(frame->tx.data, frame->tx.len, "TX Data [%dB]", frame->tx.len);
}

int udp_open(udp_sock* udp, uint16_t port) {
    struct sockaddr_in sa;
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    memset(udp, 0, sizeof(udp_sock));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;

    udp->port = port;
    udp->sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp->sd < 0) {
        LIB_LOG_ERR("socket failed on (%s:%d)",
                    inet_ntoa(sa.sin_addr), port);
        return -1;
    }

#if 0 /** Usually we cleanup, so this shouldn't be required */
    if(lib_net_reuseaddr(udp->sd) < 0) {
        LIB_LOG_ERR("set SO_REUSEADDR failed");
    }
#endif

    ret = bind(udp->sd, (struct sockaddr*)&sa, sizeof(struct sockaddr));
    if(ret < 0) {
        LIB_LOG_ERR("bind failed on %s:%d", inet_ntoa(sa.sin_addr), port);
        goto err;
    }

    if(!port) { // port == 0 auto-assigns the next available port
        ret = getsockname(udp->sd, (struct sockaddr*)&sa, &addrlen);
        if(ret < 0) {
            LIB_LOG_ERR("getsockname failed on (%s:%d)",
                        inet_ntoa(sa.sin_addr), port);
            goto err;
        }
        udp->port = ntohs(sa.sin_port);
    }

    return 0;

err:
    udp_close(udp);
    return -1;
}

void udp_close(udp_sock* udp) {
    if(udp->sd >= 0)
        close(udp->sd);

    memset(udp, 0, sizeof(udp_sock));
    udp->sd = -1;
}

int udp_poll(udp_sock* udp, int timeout) {
    struct pollfd fds;
    int ret;

    fds.fd = udp->sd;
    fds.events = (POLLIN | POLLPRI);
    ret = poll(&fds, 1, timeout);

    if(ret < 0) { // error
        LIB_LOG_ERR("poll");
        return -1;
    }

    if(ret > 0 && (fds.revents & POLLIN || fds.revents & POLLPRI)) {
        return 1;
    }

    return 0; // timeout
}

static void udp_pkt_info(lib_netpkt* frame, struct msghdr* msg) {
    struct in_pktinfo* pktinfo;
    struct cmsghdr* cmsg;

    if(msg->msg_flags & MSG_CTRUNC) {
        LIB_LOG_WARNING("control truncation detected");
        return;
    }

    for(cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg)) {
        switch(cmsg->cmsg_type) {
        case IP_TOS:
        case IP_TTL:
        case IP_HDRINCL:
        case IP_OPTIONS:
        case IP_ROUTER_ALERT:
        case IP_RECVOPTS:
        case IP_RETOPTS:
            break;
        case IP_PKTINFO:
            if(cmsg->cmsg_type == IP_PKTINFO) {
                pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
                frame->sysidx = pktinfo->ipi_ifindex;
                frame->broadcast = (pktinfo->ipi_addr.s_addr == INADDR_BROADCAST);
            }
            break;
        case IP_PKTOPTIONS:
        case IP_MTU_DISCOVER:
        case IP_RECVERR:
        case IP_RECVTTL:
        case IP_RECVTOS:
        case IP_MTU:
        case IP_FREEBIND:
        case IP_IPSEC_POLICY:
        case IP_XFRM_POLICY:
        case IP_PASSSEC:
        case IP_TRANSPARENT:
        default:
            break;
        }
    }
}

#define UDP_CTRL_BUF (128)
int udp_recv(udp_sock* udp, lib_netpkt* frame) {
    struct msghdr msg;
    struct iovec vec;
    char ctrl_buf[UDP_CTRL_BUF];
    int ret;

    if(!udp) {
        errno = EINVAL;
        return -1;
    }

    frame->broadcast = 0;
    frame->raw = 0;

    vec.iov_base = frame->rx.data;
    vec.iov_len = frame->rx.size;

    msg.msg_name = &frame->addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = ctrl_buf;
    msg.msg_controllen = UDP_CTRL_BUF;
    msg.msg_flags = 0;

    lib_buffer_clear(&frame->rx);
    ret = recvmsg(udp->sd, &msg, MSG_DONTWAIT);
    if(ret < 0) {
        if(udp->listener && errno != EAGAIN && errno != EWOULDBLOCK)
            udp->listener(udp, NULL);

        goto err;
    }

    frame->rx.len = ret;

    if(msg.msg_flags & MSG_TRUNC) {
        LIB_LOG_WARNING("datagram truncation detected");
    }

    udp_pkt_info(frame, &msg);

    if(lib_netif_has_adr(lib_netif_name(frame->sysidx), &frame->addr) &&
       frame->addr.sin_port == htons(udp->port)) {
        LIB_LOG_WARNING("broadcast recursion detected"); // sending broadcasts to myself
        goto err;
    }

    if(udp->listener) {
        ret = udp->listener(udp, frame);
    }
err:
    return ret;
}

int udp_send(udp_sock* udp, lib_netpkt* frame) {
    const uint8_t* p = NULL;
    size_t len = 0;
    int ret;

    if(!udp || !frame)
        return -EINVAL;

    if(frame->raw)
        return 0;

    if(frame->broadcast) {
        frame->addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    }

    len = frame->tx.len;
    p = (const uint8_t*)frame->tx.data;
    while(len > 0) {
        ret = sendto(udp->sd, p, len,
                     0, (struct sockaddr*)&frame->addr, sizeof(struct sockaddr));

        if(ret < 0) {
            LIB_LOG_ERR("failed to send %zu/%zuB", 
                        frame->tx.len - len, frame->tx.len);
            return -1;
        }

        p += ret;
        len -= ret;
    }

    return frame->tx.len;
}

int udp_sendto(udp_sock* udp, lib_netpkt* frame, const uint8_t* buf, size_t size) {
    const uint8_t* p = NULL;
    size_t len = 0;
    int ret;

    if(!udp || !frame)
        return -EINVAL;

    if(frame->raw)
        return 0;

    if(frame->broadcast) {
        frame->addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    }

    len = size;
    p = buf;
    while(len > 0) {
        ret = sendto(udp->sd, p, len,
                     0, (struct sockaddr*)&frame->addr, sizeof(struct sockaddr));

        if(ret < 0) {
            LIB_LOG_ERR("failed to send %zu/%zuB", 
                        size - len, len);
            return -1;
        }

        p += ret;
        len -= ret;
    }

    return size;
}

int udp_flush(udp_sock* udp) {
    int ret, queue;

    if(!udp || udp->sd == -1)
        return 0;

    while(1) {
        ret = ioctl(udp->sd, TIOCOUTQ, &queue);
        if(ret != 0) {
            LIB_LOG_ERR("set TIOCOUTQ");
            return -1;
        }

        if(!queue) {
            return 0;
        }

        usleep(100);
    }
    return 0;
}