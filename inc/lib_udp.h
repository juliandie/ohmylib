
#ifndef UDP_SOCK_H_
#define UDP_SOCK_H_

#include <stdint.h>

#include <arpa/inet.h>

#include <lib_netif.h>

typedef struct udp_sock_s {
    int sd;

    /** In case the socket was opened with a 0 port
     * the assigned port number can be read back
     */
    uint16_t port;

    /** The listener callback shall be used by server applications */
    int (*listener)(struct udp_sock_s *, struct lib_netpkt_s *);
} udp_sock;

void udp_dump_rx_frame(lib_netpkt* frame);
void udp_dump_tx_frame(lib_netpkt* frame);

int udp_open(udp_sock* udp, uint16_t port);
void udp_close(udp_sock* udp);

int udp_poll(udp_sock* udp, int timeout);
int udp_recv(udp_sock* udp, lib_netpkt* frame);
int udp_send(udp_sock* udp, lib_netpkt* frame);
int udp_sendto(udp_sock* udp, lib_netpkt* frame, const uint8_t* buf, size_t size);
int udp_flush(udp_sock* udp);

#endif
