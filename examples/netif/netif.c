#include <sys/types.h>
#include <ifaddrs.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h> /** if_indextoname */

static void netif_dmp(const char *ifname) {
    struct if_hwaddr hwaddr;
    struct in_addr addr;
    int cnt;

    addr.s_addr = htonl(lib_netif_def_gtw(ifname));
    printf("%s default gateway - %s\n", ifname, inet_ntoa(addr));

    printf("Interface flags: 0x%08x\n", lib_netif_flags(ifname));

    cnt = lib_netif_adr_cnt(ifname);
    for(int i = 0; i < cnt; i++) {
        uint32_t spd, mtu;
        lib_netif_hwaddr(ifname, &hwaddr);
        printf("%s(%d) (%d - %s)\n",
               ifname, i, lib_netif_idx(ifname), lib_hwaddrtos(&hwaddr));
        addr.s_addr = htonl(lib_netif_adr_idx(ifname, i));
        printf("    adr: %s\n", inet_ntoa(addr));
        addr.s_addr = htonl(lib_netif_dst_idx(ifname, i));
        printf("    dst: %s\n", inet_ntoa(addr));
        addr.s_addr = htonl(lib_netif_sub_idx(ifname, i));
        printf("    sub: %s\n", inet_ntoa(addr));
        addr.s_addr = htonl(lib_netif_brd_idx(ifname, i));
        printf("    brd: %s\n", inet_ntoa(addr));
        spd = lib_netif_speed(ifname);
        printf("    spd: %ub/s\n", spd);
        mtu = lib_netif_mtu(ifname);
        printf("    mtu: %uB\n", mtu);
        printf("\n");
    }
}

static int netif_ioctl(const char *ifname, int code, struct ifreq *ifr) {
    int  ret;

    if(ifname == NULL) {
        errno = -EINVAL;
        return -1;
    }

    int sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if(sock < 0) {
        return -1;
    }

    strncpy(ifr->ifr_name, ifname, sizeof(ifr->ifr_name) - 1);
    ret = ioctl(sock, code, ifr);
    close(sock);
    return ret;
}

int netif_flags(const char *ifname) {
    struct ifreq ifr;
    if(netif_ioctl(ifname, SIOCGIFFLAGS, &ifr) == -1) {
        fprintf("failed ioctl(SIOCGIFFLAGS): %s\n", strerror(errno));
        return -1;
    }

    return ifr.ifr_flags;
}

int netif_interfaces(const char *ifnames) {
    char *str, *n;
    struct ifaddrs *ifaddr, *ifa;
    char **buf;
    int ret, n = 1;

    iter_substr(str, ifnames, n, ',') {
    }

    ret = getifaddrs(&ifaddr);
    if(ret) {
        fprintf(stderr, "getifaddr\n");
        return -1;
    }

    buf = calloc(n, sizeof(char *));

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;

        if(ifa->ifa_addr->sa_family == AF_INET) {
            int skip = 0;

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

int main(int argc, char **argv) {
    int ret;

    if(argc < 2) {
        fprintf(stderr, "usage: %s <netif0>[,<netif1>,...]\n", argv[0]);
        return -1;
    }

    netif_interfaces(argv[1]);
    return 0;
}
