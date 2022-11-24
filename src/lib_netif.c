
#include <lib_netif.h>

#include <stdlib.h> /** malloc, free */
#include <unistd.h> /** close */
#include <string.h> /** strcmp */
#include <stdint.h> /** uint_* */
#include <ifaddrs.h> /** struct ifaddrs */
#include <net/if.h> /** if_indextoname */
#include <net/if_arp.h> /** arpreq */
#include <linux/sockios.h> /** SIOCETHTOOL */
#include <linux/ethtool.h> /** ethtool_cmd */
#include <linux/route.h> /** rtentry */
#include <sys/ioctl.h> /** ioctl */

#if 0
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
#endif

#include <lib_log.h>

static int lib_netif_ioctl(const char *ifname, int code, struct ifreq *ifr) {
    int sock4, ret;

    if(ifname == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /*AF_INET - to define network interface IPv4*/
    /*Creating soket for it.*/
    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    //sock6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sock4 < 0)
        return -1;

    strncpy(ifr->ifr_name, ifname, sizeof(ifr->ifr_name) - 1);
    ret = ioctl(sock4, code, ifr);
    close(sock4);

    return ret;
}

int lib_netif_hwaddr(const char *ifname, uint8_t *buf) {
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFHWADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("%s SIOCGIFHWADDR", ifname);
        return -1;
    }

    if(ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
        LIB_LOG_DEBUG("%s ARPHRD_ETHER", ifname);
        return -1;
    }

    if(buf != NULL) { // only check for valid MAC
        memcpy(buf, ifr.ifr_hwaddr.sa_data, 6);
    }
    return 0;
}

int lib_netif_has_adr(const char *ifname, const struct sockaddr_in *ina) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0;

    if(!ifname)
        return 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return 0;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        ret = (((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr == ina->sin_addr.s_addr);
        if(ret)
            goto done;
    }

done:
    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_adr_cnt(const char *ifname) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        ret++;
    }

    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_adr_idx(const char *ifname, int idx) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0, i = 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        if(i++ != idx)
            continue;

        ret = ntohl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);
        goto done;
    }

    LIB_LOG_WARNING("no such address (%s:%d)", ifname, idx);
done:
    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_adr(const char *ifname) {
#if 0 // I can't identify mutiples IPs, using the IOCTL if
    struct ifreq ifr;
    int ret = 0;

    if(lib_netif_ioctl(ifname, SIOCGIFADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFADDR");
        return -1;
    }

    switch(ifr.ifr_addr.sa_family) {
    case AF_INET:
        ret = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        break;
#if 0
    case AF_INET6:
        memcy(buf, ((struct sockaddr_in6 *)&ifr.ifr_addr)->sin_addr.s_addr, 16);
        break;
#endif
    default:
        LIB_LOG_WARNING("Invalid address family");
        ret = -1;
    }

    return ret;
#else
    return lib_netif_adr_idx(ifname, 0);
#endif
}

int lib_netif_adr_add(const char *ifname, int adr) {
    struct ifreq ifr;

    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = htonl(adr);

    if(lib_netif_ioctl(ifname, SIOCSIFADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCSIFADDR");
        return -1;
    }

    return 0;
}

int lib_netif_route_add(const char *ifname, int dst, int sub, int gtw) {
    struct rtentry rt = {};
    int sock4, ret = -1;

    if(ifname == NULL)
        return -1;

    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock4 < 0)
        return -1;

    /** Gateway */
    ((struct sockaddr_in *)&(rt.rt_gateway))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(rt.rt_gateway))->sin_addr.s_addr = htonl(gtw);

    /** IP Address */
    ((struct sockaddr_in *)&(rt.rt_dst))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(rt.rt_dst))->sin_addr.s_addr = htonl(dst);

    /** Subnet mask */
    ((struct sockaddr_in *)&(rt.rt_genmask))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(rt.rt_genmask))->sin_addr.s_addr = htonl(sub);

    rt.rt_flags = RTF_UP | RTF_GATEWAY;
    rt.rt_dev = strdup(ifname);
    if(!rt.rt_dev) {
        goto err_strdup;
    }

    ret = ioctl(sock4, SIOCADDRT, &rt);
    if(ret == -1) {
        if(errno != EEXIST) {
            LIB_LOG_DEBUG("SIOCADDRT");
        }
        else { // Route is already set
            ret = 0;
        }
    }

    free(rt.rt_dev);
err_strdup:
    close(sock4);
    return ret;
}

int lib_netif_def_gtw(const char *ifname) {
    FILE *f;
    int ret = 0, res;
    uint32_t dst, gtw, msk, first = 0;
    char ifn[IFNAMSIZ];

    f = fopen("/proc/net/route", "r");
    if(f == NULL) {
        LIB_LOG_DEBUG("failed to open /proc/net/route");
        return -1;
    }

    res = fscanf(f, "%*[^\n]\n"); // discard header
    if(res == EOF && ferror(f)) {
        LIB_LOG_DEBUG("error reading from /proc/net/route");
        goto close_file;
    }

    if(res) {
        LIB_LOG_WARNING("format error reading from /proc/net/route (%i)", res);
        goto close_file;
    }

    // iface dst gtw flags refcnt use metric mask mtu window irtt
    // %32s  %x  %x  %*x   %*d    %*d %*d    %x   %*[^\n]
    while(fscanf(f, "%32s %x %x %*x %*d %*d %*d %x %*[^\n]\n",
                 ifn, &dst, &gtw, &msk) == 4) {
        if(dst != 0)
            continue;

        if(msk != 0)
            continue;

        if(!first)
            first = gtw;

        if(strcmp(ifname, ifn))
            continue;

        ret = gtw;
        break;
    }

    if(!ret)
        ret = first;

close_file:
    fclose(f);
    return ntohl(ret);
}

int lib_netif_dst_idx(const char *ifname, int idx) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0, i = 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        if(i++ != idx)
            continue;

        ret = ntohl(((struct sockaddr_in *)ifa->ifa_dstaddr)->sin_addr.s_addr);
        goto done;
    }

    LIB_LOG_WARNING("no such destination (%s:%d)", ifname, idx);
done:
    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_dst(const char *ifname) {
#if 0 // I can't identify mutiples IPs, using the IOCTL if
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFDSTADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFDSTADDR");
        return -1;
    }

    if(ifr.ifr_dstaddr.sa_family != AF_INET) {
        LIB_LOG_DEBUG("AF_INET");
        return -1;
    }

    return ((struct sockaddr_in *)(&ifr.ifr_dstaddr))->sin_addr.s_addr;
#else
    return lib_netif_dst_idx(ifname, 0);
#endif
}

int lib_netif_brd_idx(const char *ifname, int idx) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0, i = 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        if(i++ != idx)
            continue;

        ret = ntohl(((struct sockaddr_in *)ifa->ifa_broadaddr)->sin_addr.s_addr);
        goto done;
    }

    LIB_LOG_WARNING("no such broadcast (%s:%d)", ifname, idx);
done:
    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_brd(const char *ifname) {
#if 0 // I can't identify mutiples IPs, using the IOCTL if
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFBRDADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFBRDADDR");
        return -1;
    }

    if(ifr.ifr_broadaddr.sa_family != AF_INET) {
        LIB_LOG_DEBUG("AF_INET");
        return -1;
    }

    return ((struct sockaddr_in *)(&ifr.ifr_broadaddr))->sin_addr.s_addr;
#else
    return lib_netif_brd_idx(ifname, 0);
#endif
}

int lib_netif_brd_add(const char *ifname, int brd) {
    struct ifreq ifr;

    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = htonl(brd);

    if(lib_netif_ioctl(ifname, SIOCSIFDSTADDR, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCSIFDSTADDR");
        return -1;
    }

    return 0;
}

int lib_netif_sub_idx(const char *ifname, int idx) {
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0, i = 0;

    if(getifaddrs(&ifaddr)) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if(strcmp(ifa->ifa_name, ifname))
            continue;

        if(i++ != idx)
            continue;

        ret = ntohl(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr);
        goto done;
    }

    LIB_LOG_WARNING("no such subnet (%s:%d)", ifname, idx);
done:
    freeifaddrs(ifaddr);
    return ret;
}

int lib_netif_sub(const char *ifname) {
#if 0 // I can't identify mutiples IPs, using the IOCTL if
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFNETMASK, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFNETMASK");
        return -1;
    }

    if(ifr.ifr_netmask.sa_family != AF_INET) {
        LIB_LOG_DEBUG("AF_INET");
        return -1;
    }

    return ((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr.s_addr;
#else
    return lib_netif_sub_idx(ifname, 0);
#endif
}

int lib_netif_sub_add(const char *ifname, int sub) {
    struct ifreq ifr;

    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = htonl(sub);

    if(lib_netif_ioctl(ifname, SIOCSIFNETMASK, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCSIFNETMASK");
        return -1;
    }

    return 0;
}

int lib_netif_speed(const char *ifname) {
    struct ifreq ifr;
#if 0 // DEPRECATED ethtool
    struct ethtool_cmd edata;
    memset(&edata, 0, sizeof(struct ethtool_cmd));
    edata.cmd = ETHTOOL_GSET;
#else
    struct ethtool_link_settings edata;
    memset(&edata, 0, sizeof(struct ethtool_link_settings));
    edata.cmd = ETHTOOL_GLINKSETTINGS;
#endif
    ifr.ifr_data = (char *)&edata;
    if(lib_netif_ioctl(ifname, SIOCETHTOOL, &ifr) < 0) {
        LIB_LOG_DEBUG("SIOCETHTOOL (%s)", ifname);
        return -1;
    }

    return edata.speed;
}

#if 0
enum net_device_flags {
    IFF_UP = 1 << 0,  /* sysfs */
    IFF_BROADCAST = 1 << 1,  /* volatile */
    IFF_DEBUG = 1 << 2,  /* sysfs */
    IFF_LOOPBACK = 1 << 3,  /* volatile */
    IFF_POINTOPOINT = 1 << 4,  /* volatile */
    IFF_NOTRAILERS = 1 << 5,  /* sysfs */
    IFF_RUNNING = 1 << 6,  /* volatile */
    IFF_NOARP = 1 << 7,  /* sysfs */
    IFF_PROMISC = 1 << 8,  /* sysfs */
    IFF_ALLMULTI = 1 << 9,  /* sysfs */
    IFF_MASTER = 1 << 10, /* volatile */
    IFF_SLAVE = 1 << 11, /* volatile */
    IFF_MULTICAST = 1 << 12, /* sysfs */
    IFF_PORTSEL = 1 << 13, /* sysfs */
    IFF_AUTOMEDIA = 1 << 14, /* sysfs */
    IFF_DYNAMIC = 1 << 15, /* sysfs */
    IFF_LOWER_UP = 1 << 16, /* volatile */
    IFF_DORMANT = 1 << 17, /* volatile */
    IFF_ECHO = 1 << 18, /* volatile */
};
#endif
int lib_netif_flags(const char *ifname) {
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFFLAGS, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFFLAGS");
        return -1;
    }

    return ifr.ifr_flags;
}

static int lib_netif_flags_set(const char *ifname, int flags) {
    struct ifreq ifr;

    ifr.ifr_flags = (short)flags;

    if(lib_netif_ioctl(ifname, SIOCSIFFLAGS, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCSIFFLAGS");
        return -1;
    }

    return 0;
}

int lib_netif_mtu(const char *ifname) {
    struct ifreq ifr;

    if(lib_netif_ioctl(ifname, SIOCGIFMTU, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCGIFMTU");
        return -1;
    }

    return ifr.ifr_mtu;
}

int lib_netif_mtu_set(const char *ifname, int mtu) {
    struct ifreq ifr;

    if(lib_netif_down(ifname) < 0)
        return -1;

    ifr.ifr_mtu = mtu;

    if(lib_netif_ioctl(ifname, SIOCSIFMTU, &ifr) == -1) {
        LIB_LOG_DEBUG("SIOCSIFMTU");
        if(lib_netif_up(ifname) < 0)
            return -1;

        return -1;
    }

    if(lib_netif_up(ifname) < 0)
        return -1;

    return ifr.ifr_mtu;
}

int lib_netif_idx(const char *ifname) {
    struct ifreq req;

    if(lib_netif_ioctl(ifname, SIOCGIFINDEX, &req) < 0) {
        LIB_LOG_DEBUG("SIOCGIFINDEX");
        return -1;
    }

    return req.ifr_ifindex;
}

static char lib_netif_ifname[IF_NAMESIZE]; // like inet_ntoa
char *lib_netif_name(int idx) {
    if(if_indextoname(idx, lib_netif_ifname) == NULL) {
        return NULL;
    }

    return lib_netif_ifname;
}

static  __attribute__((unused)) int lib_netif_get_from_arp_table(const char *ifname, uint32_t ip, uint8_t *mac) {
    struct sockaddr_in *reqaddr;
    struct arpreq req;
    int sock4, ret;

    if(ifname == NULL)
        return -1;

    sock4 = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock4 < 0)
        return -1;

    memset(&req, 0, sizeof(struct arpreq));

    reqaddr = (struct sockaddr_in *)&req.arp_pa;
    reqaddr->sin_family = AF_INET;
    reqaddr->sin_addr.s_addr = ip;
    strncpy(req.arp_dev, ifname, IFNAMSIZ - 1);
    req.arp_ha.sa_family = AF_UNSPEC;

    ret = ioctl(sock4, SIOCGARP, &req);
    close(sock4);

    if(req.arp_flags & ATF_COM) {
        memcpy(mac, req.arp_ha.sa_data, ETH_ALEN);
    }

    return ret;
}

int sys_netif_get_next_hop_mac(const char *netif, uint32_t dest_ip, uint8_t *next_hop_mac, uint8_t *ttl) {
#if 0
    char cmd[MAX_CMD];
    int result;
    struct in_addr dest_addr;
    FILE *iproute;
    char rdbuf[25];
    enum {
        none,
        via,
        hoplimit
    } key = none;
    uint8_t broadcast = 0;
    uint8_t multicast = 0;

    dest_addr.s_addr = htonl(dest_ip);
    *ttl = IPDEFTTL;

    // Build ip route command line
    result = snprintf(cmd, MAX_CMD, "/bin/ip route get to %s oif %s", inet_ntoa(dest_addr), ifnames[index]);
    if(result >= MAX_CMD) {
        fprintf(stderr, "gigevd: sys_netif_get_next_hop_mac: command buffer for ip route overflowed\n");
        return -1;
    }
    else if(result < 0) {
        warn("sys_netif_get_next_hop_mac: error creating command string for ip route");
        return -1;
    }

    // Execute ip route command
    iproute = popen(cmd, "r");
    if(iproute == NULL) {
        warn("sys_netif_get_next_hop_mac: error executing %s", cmd);
        return -1;
    }

    // Parse its standard output to find:
    // - broadcast/multicast indication
    // - gateway IP address to use (via)
    // - TTL (hoplimit)
    while(fscanf(iproute, "%24s", rdbuf) == 1) {
        switch(key) {
        case none:
            if(strcmp(rdbuf, "broadcast") == 0) {
                broadcast = 1;
            }
            else if(strcmp(rdbuf, "multicast") == 0) {
                multicast = 1;
            }
            else if(strcmp(rdbuf, "via") == 0) {
                key = via;
            }
            else if(strcmp(rdbuf, "hoplimit") == 0) {
                key = hoplimit;
            }
            break;

        case via:
            // Gateway IP address
            if(inet_aton(rdbuf, &dest_addr) == 0) {
                // IP address invalid
                fprintf(stderr, "gigevd: sys_netif_get_next_hop_mac: command '%s' returned invalid 'via' IP address: %s", cmd, rdbuf);
                dest_addr.s_addr = htonl(dest_ip);
            }
            key = none;
            break;

        case hoplimit:
            // TTL
            *ttl = strtoul(rdbuf, NULL, 0);
            if(*ttl == 0) {
                *ttl = IPDEFTTL;
            }
            key = none;
            break;
        }
    }

    result = pclose(iproute);
    if(result == -1) {
        warn("sys_netif_get_next_hop_mac: pclose failed");
    }

    if(broadcast) {
        // Broadcast always goes to FF:FF:FF:FF:FF:FF
        memset(next_hop_mac, 0xFF, ETH_ALEN);
        return 0;
    }

    if(multicast) {
        // Calculate multicast MAC address according to RFC1112
        uint32_t dest_ip_h = ntohl(dest_addr.s_addr);
        next_hop_mac[0] = 0x01;
        next_hop_mac[1] = 0x00;
        next_hop_mac[2] = 0x5E;
        next_hop_mac[3] = ((dest_ip_h >> 16) & 0x7F);
        next_hop_mac[4] = ((dest_ip_h >> 8) & 0xFF);
        next_hop_mac[5] = (dest_ip_h & 0xFF);
        return 0;
    }

    // Resolve MAC address using ARP
    result = netif_get_from_arp_table(netif, dest_addr.s_addr, next_hop_mac);
    if(result == -2) {
        return -1;
    }
    else if(result == -1) {
        // IP address not found in ARP table
        // -> send ARP request and then try again

        // Build arping command line
        result = snprintf(cmd, MAX_CMD, "/usr/bin/arping -c 3 -f -I %s %s", ifnames[index], inet_ntoa(dest_addr));
        if(result >= MAX_CMD) {
            fprintf(stderr, "gigevd: sys_netif_get_next_hop_mac: command buffer for arping overflowed\n");
            return -1;
        }
        else if(result < 0) {
            warn("sys_netif_get_next_hop_mac: error creating command string for arping");
            return -1;
        }

        // Execute arping command
        result = system(cmd);
        if(result == -1) {
            warn("sys_netif_get_next_hop_mac: error executing %s", cmd);
            return -1;
        }
        if(WEXITSTATUS(result) != 0) {
            fprintf(stderr, "gigevd: sys_netif_get_next_hop_mac: %s failed\n", cmd);
        }

        // Retry getting resolved MAC address from ARP table
        result = netif_get_from_arp_table(netif, dest_addr.s_addr, next_hop_mac);
    }

    if(result == 0) {
        return 0;
    }
    else {
        return -1;
    }
#else
    (void)netif;
    (void)dest_ip;
    (void)next_hop_mac;
    (void)ttl;
    return 0;
#endif
}

#if 0
static __attribute__((unused)) void *lib_netif_ifupdown(void *p) {
    int ret;
    lib_netif *netif = (lib_netif *)p;

    /** TODO do thread funcies */
    ret = lib_netif_flags(netif->ifname);
    if(netif->state == 0 && (ret & IFF_UP || ret & IFF_RUNNING)) {
        if(netif->link_up != NULL)
            netif->link_up(netif);
    }
    else {
        if(netif->link_down != NULL)
            netif->link_down(netif);
    }

    return NULL;
#if 0 // the fancy way
    // http://man7.org/linux/man-pages/man7/netlink.7.html
    // https://www.infradead.org/~tgr/libnl/
    const int netlink_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(netlink_fd != -1) {
        struct sockaddr_nl sa;

        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        sa.nl_groups = RTNLGRP_LINK;
        bind(netlink_fd, (struct sockaddr *)&sa, sizeof(sa));
    }
#endif
}
#endif

int lib_netif_is_up(const char *ifname) {
    int flags;

    flags = lib_netif_flags(ifname);

    if(flags == -1) {
        return -1;
    }

    if(flags & IFF_RUNNING) {
        LIB_LOG_INFO("IFF_RUNNING");
        return 1;
    }

    return 0;
}

int lib_netif_down(const char *ifname) {
    return lib_netif_flags_set(ifname, !IFF_UP);
}

int lib_netif_up(const char *ifname) {
    return lib_netif_flags_set(ifname, IFF_UP);
}

static int lib_arr_append(const char *ifa, char ***buf, int *count) {
    char **tmp;

    tmp = realloc(*buf, sizeof(char *) * (*count + 1));
    if(tmp == NULL) {
        LIB_LOG_DEBUG("realloc");
        return -1;
    }

    tmp[*count - 1] = strdup(ifa);
    tmp[*count] = NULL;
    *count += 1;
    *buf = tmp;
    return 0;
}

int lib_netif_interfaces(char ***ifnames) {
    struct ifaddrs *ifaddr, *ifa;
    char **buf;
    int ret, n = 1;

    ret = getifaddrs(&ifaddr);
    if(ret) {
        LIB_LOG_DEBUG("getifaddr");
        return -1;
    }

    buf = calloc(n, sizeof(char *));

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        int skip = 0;
        if(ifa->ifa_addr->sa_family == AF_INET) {
            for(int i = 0; (buf)[i] != NULL; i++) {
                if(!strcmp((buf)[i], ifa->ifa_name))
                    skip = 1;
            }

            if(!skip)
                lib_arr_append(ifa->ifa_name, &buf, &n);
        }
    }

    freeifaddrs(ifaddr);

    if(n == 1 || !ifnames)
        lib_netif_interfaces_free(buf);

    if(ifnames)
        *ifnames = buf;

    return n - 1;
}

void lib_netif_interfaces_free(char **ifnames) {
    if(ifnames == NULL)
        return;

    for(int i = 0; ifnames[i] != NULL; i++)
        free(ifnames[i]);

    free(ifnames);
}

#if 0
static int netif_down(const char *netif) {
    pid_t pid = 0;
    char filename[MAX_CMD];
    FILE *fp;
    int result = 0;
    int i;

    // Build ifplugd PID file name
    snprintf(filename, MAX_CMD, "/var/run/ifplugd.%s.pid", netif);
    fp = fopen(filename, "r");
    if(fp != NULL) {
        if(fscanf(fp, "%i", &pid) != 1) {
            pid = 0;
        }
        fclose(fp);
    }

    if(pid > 0) {
        // Send SIGTERM to ifplugd
        if(kill(pid, SIGTERM) != 0) {
            warn("sys_netif_down: error killing ifplugd for %s (pid=%i)",
                 netif, pid);
            result = -1;
        }
        else {
            // Wait for the interface to go down
            for(i = 0; i < 30; ++i) {
                sys_netif_check_notifications();
                if(!(net_ifs_up & (1 << index))) {
                    break;
                }
                usleep(100000);
            }

            // Wait for the ifplugd process to terminate
            // (detected by checking when the PID file is removed)
            for(i = 0; i < 30; ++i) {
                if(access(filename, F_OK) != 0) {
                    break;
                }
                usleep(100000);
            }
        }
    }

    return result;
}

static int sys_netif_up(const char *netif) {
    char cmd[MAX_CMD];
    int result;

    // Build ifplugd command line
    result = snprintf(cmd, MAX_CMD, "/usr/bin/ifplugd -f -i %s -I -u 0 -d 0",
                      netif);
    if(result >= MAX_CMD) {
        fprintf(stderr,
                "gigevd: sys_netif_up: command buffer for ifplugd overflowed\n");
        return -1;
    }
    else if(result < 0) {
        warn("sys_netif_up: error creating command string for ifplugd");
        return -1;
    }

    // Execute ifplugd command
    result = system(cmd);
    if(result == -1) {
        warn("sys_netif_up: error executing %s", cmd);
        return -1;
    }
    if(WEXITSTATUS(result) != 0) {
        fprintf(stderr, "gigevd: sys_netif_up: %s failed\n", cmd);
        return -1;
    }

    return 0;
}

int netif_get_supported_ipconfig(const char *netif) {
    (void)netif;
    return GEV_BV32(GEV_IPCONFIG_LLA) |
        GEV_BV32(GEV_IPCONFIG_DHCP) |
        (sys_netcfg_persistent_supported() ?
         GEV_BV32(GEV_IPCONFIG_PERSISTENT_IP) : 0);
}

/**
* Forces the specified interface to use the specified
* IP address, subnet mask and default gateway in host byte order.
* \return 0 on success, -1 on failure.
*/
int sys_netif_forceip(const char *netif, uint32_t ip, uint32_t subnetmask, uint32_t defgateway) {
    int result;
    NetCfg forced_cfg;
    int i;

    if(sys_netif_down(netif) != 0) {
        return -1;
    }

    forced_cfg.meth_persistent_en = 1;
    forced_cfg.meth_dhcp_en = 0;
    forced_cfg.ipaddr = ip;
    forced_cfg.subnetmask = subnetmask;
    forced_cfg.defgateway = defgateway;

    result = sys_netcfg_set_config(netif, &forced_cfg, 0);

    if(result == 0) {
        net_ip_config_methods = IPCONF_FORCE;
    }

    if(sys_netif_up(netif) != 0) {
        return -1;
    }

    if(result != 0) {
        return -1;
    }

    // Wait for the interface to come up
    for(i = 0; i < 30; ++i) {
        usleep(500000);
        sys_netif_check_notifications();
        if(net_ifs_up & (1 << index)) {
            break;
        }
    }

    return result;
}

/**
* Restarts the IP configuration cycle on all interfaces.
* \return 0 on success, nonzero on failure.
*/
int sys_netif_restart_ipconfig() {
    int i;
    int result = 0;
    NetCfg cfg;

    for(i = 0; i < sys_netif_get_count(); ++i) {
        if(sys_netcfg_get_config(ifnames[i], &cfg) != 0) {
            continue;
        }

        if(sys_netif_down(i) != 0) {
            continue;
        }

        // Make sure the most recent permanently stored configuration is copied
        // to the ramdisk.
        if(sys_netcfg_set_config(ifnames[i], &cfg, 0) != 0) {
            result = -1;
        }

        if(sys_netif_up(i) != 0) {
            result = -1;
        }
    }

    return result;
}

#endif

int lib_net_init_netpkt_sync(lib_netpkt *frame, int size) {
    memset(frame, 0, sizeof(lib_netpkt));
    if(lib_buffer_alloc(&frame->rx, size) < 0)
        return -1;

    if(lib_buffer_alloc(&frame->tx, size) < 0) {
        lib_buffer_free(&frame->rx);
        return -1;
    }

    return 0;
}

int lib_net_init_netpkt_async(lib_netpkt *frame, int rx_size, int tx_size) {
    memset(frame, 0, sizeof(lib_netpkt));
    if(lib_buffer_alloc(&frame->rx, rx_size) < 0)
        return -1;

    if(lib_buffer_alloc(&frame->tx, tx_size) < 0) {
        lib_buffer_free(&frame->rx);
        return -1;
    }

    return 0;
}

void lib_net_free_netpkt(lib_netpkt *frame) {
    lib_buffer_free(&frame->tx);
    lib_buffer_free(&frame->rx);
}

static int lib_net_setsockopt(int sd, int lvl, int optname, int optval) {
    return setsockopt(sd, lvl, optname, &optval, sizeof(int));
}
static int lib_net_getsockopt(int sd, int lvl, int optname) {
    uint32_t len = 4;
    int val;
    if(getsockopt(sd, lvl, optname, &val, &len))
        return -1;

    return val;
}

int lib_net_reuseaddr(int sd) {
    return lib_net_setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 1);
}

int lib_net_disable_fragment(int sd) {
#ifdef IP_PMTUDISC_PROBE
    return lib_net_setsockopt(sd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_PROBE);
#else
    return lib_net_setsockopt(sd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_DO);
#endif
}

int lib_net_enable_fragment(int sd) {
    return lib_net_setsockopt(sd, SOL_IP, IP_MTU_DISCOVER, IP_PMTUDISC_DONT);
}

int lib_net_is_fragmented(int sd) {
    return lib_net_getsockopt(sd, SOL_IP, IP_MTU_DISCOVER);
}

int lib_net_enable_pktinfo(int sd) {
    return lib_net_setsockopt(sd, SOL_IP, IP_PKTINFO, 1);
}

int lib_net_enable_broadcast(int sd) {
    return lib_net_setsockopt(sd, SOL_SOCKET, SO_BROADCAST, 1);
}

int lib_net_bind_to_if(int sd, const char *ifname) {
    return setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname));
}
