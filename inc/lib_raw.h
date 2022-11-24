
#ifndef RAW_SOCK_H_
#define RAW_SOCK_H_

#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lib_buffer.h>
#include <lib_netif.h>

typedef struct raw_sock_s {
    int sd;
    int sysidx;
    char* ifname;

    uint16_t ip_id; /**< Next IP packet id, used for raw packets only */
    lib_netpkt frame;

    int (*listener)(struct raw_sock_s*, struct lib_netpkt_s*);
} raw_sock;

raw_sock* raw_create(const char* ifname);
int raw_open(raw_sock* raw, const char* ifname);
void raw_close(raw_sock* raw);
void raw_destroy(raw_sock* raw);

int raw_poll(raw_sock* raw, int timeout);
int raw_recv(raw_sock* raw);
int raw_udp_send(raw_sock* raw, lib_netpkt *frame);

#endif

