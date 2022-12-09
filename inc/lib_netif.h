
#ifndef LIB_NETIF_H_
#define LIB_NETIF_H_

//#include <stdint.h> /** uint_* */
#include <arpa/inet.h>
#include <net/ethernet.h>
//#include <net/if_arp.h>
//#include <netinet/if_ether.h>

struct if_hwaddr {
    char mac[ETH_ALEN];
};

/** MAC */
int lib_netif_hwaddr(const char *ifname, struct if_hwaddr *addr);
const char *lib_hwaddrtos(struct if_hwaddr *addr);
/** IP */
int lib_netif_has_adr(const char *ifname, const struct sockaddr_in *ina);
int lib_netif_adr_cnt(const char *ifname);
int lib_netif_adr_idx(const char *ifname, int idx);
int lib_netif_adr(const char *ifname);
int lib_netif_adr_add(const char *ifname, int adr);
/** Routing */
int lib_netif_route_add(const char *ifname, int dst, int sub, int gtw);
int lib_netif_def_gtw(const char *ifname);
int lib_netif_dst_idx(const char *ifname, int idx);
int lib_netif_dst(const char *ifname);
/** Broadcast */
int lib_netif_brd_idx(const char *ifname, int idx);
int lib_netif_brd(const char *ifname);
int lib_netif_brd_add(const char *ifname, int brd);
/** Subnetmask */
int lib_netif_sub_idx(const char *ifname, int idx);
int lib_netif_sub(const char *ifname);
int lib_netif_sub_add(const char *ifname, int sub);
/** If info */
int lib_netif_speed(const char *ifname);
int lib_netif_flags(const char *ifname);
int lib_netif_mtu(const char *ifname);
int lib_netif_mtu_set(const char *ifname, int mtu);

int lib_netif_idx(const char *ifname);
char *lib_netif_name(int idx);

int sys_netif_get_next_hop_mac(const char *netif, uint32_t dst, uint8_t *hop, uint8_t *ttl);
int lib_netif_is_up(const char *ifname);
int lib_netif_down(const char *ifname);
int lib_netif_up(const char *ifname);

int lib_netif_interfaces(char ***ifnames);
void lib_netif_interfaces_free(char **ifnames);

#endif

