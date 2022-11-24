
#ifndef LIB_NETIF_H_
#define LIB_NETIF_H_

//#include <stdint.h> /** uint_* */
#include <arpa/inet.h>
#include <lib_buffer.h>

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
#endif

typedef struct lib_netpkt_s {
    int sysidx;
    int broadcast;
    int raw;

    struct sockaddr_in addr;
    lib_buffer rx;
    lib_buffer tx;
} lib_netpkt;

/** MAC */
int lib_netif_hwaddr(const char *ifname, uint8_t *buf);
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

int lib_net_init_netpkt_sync(lib_netpkt *frame, int size);
int lib_net_init_netpkt_async(lib_netpkt *frame, int rx_size, int tx_size);
void lib_net_free_netpkt(lib_netpkt *frame);

int lib_net_reuseaddr(int sd);
int lib_net_disable_fragment(int sd);
int lib_net_enable_fragment(int sd);
int lib_net_is_fragmented(int sd);
int lib_net_enable_pktinfo(int sd);
int lib_net_enable_broadcast(int sd);
int lib_net_bind_to_if(int sd, const char *ifname);

#endif

