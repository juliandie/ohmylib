
#include <sys/types.h>
#include <ifaddrs.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h> /** if_indextoname */

#include <lib_netif.h>
#include <lib_log.h>

static void netif_dmp(const char* ifname) {
    uint8_t mac[6] = {};
    struct in_addr addr;
    uint32_t spd, mtu;
    int cnt;

    addr.s_addr = htonl(lib_netif_def_gtw(ifname));
    printf("%s default gateway - %s\n", ifname, inet_ntoa(addr));

    printf("Interface flags: 0x%08x\n", lib_netif_flags(ifname));

    cnt = lib_netif_adr_cnt(ifname);
    for (int i = 0; i < cnt; i++) {

        lib_netif_hwaddr(ifname, mac);
        printf("%s(%d) (%d - %02x:%02x:%02x:%02x:%02x:%02x)\n",
            ifname, i, lib_netif_idx(ifname),
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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

int main() {
    char** buf = NULL;
    int ret;

    printf("Interface flags: 0x%08x\n", lib_netif_flags("eth1"));

    ret = lib_netif_interfaces(&buf);
    if (ret < 0) {
        return ret;
    }
    if (ret == 0) {
        LIB_LOG_INFO("no network interfaces");
        return 0;
    }
    for (int i = 0; i < ret; i++) {
        netif_dmp(buf[i]);
    }

    lib_netif_interfaces_free(buf);
    return 0;
}
