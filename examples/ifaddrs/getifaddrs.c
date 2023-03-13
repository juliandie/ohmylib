#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

int main(/*int argc, char *argv[]*/) {
    struct ifaddrs *ifaddr;
    struct sockaddr *addr;
    int s;
    char host[NI_MAXHOST];

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we can free list later */
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL;
        ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;

        addr = ifa->ifa_addr;

        /* Display interface name and family (including symbolic
           form of the latter for the common families) */

        switch(addr->sa_family) {
        case AF_PACKET:
            printf("%-8s %s (%d)\n", ifa->ifa_name, "AF_PACKET", addr->sa_family);
            break;
        case AF_INET:
            printf("%-8s %s (%d)\n", ifa->ifa_name, "AF_INET", addr->sa_family);
            break;
        case AF_INET6:
            printf("%-8s %s (%d)\n", ifa->ifa_name, "AF_INET6", addr->sa_family);
            break;
        default:
            printf("%-8s %s (%d)\n", ifa->ifa_name, "???", addr->sa_family);
            break;
        }

        /* For an AF_INET* interface address, display the address */

        if(addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST,
                            NULL, 0, NI_NUMERICHOST);
            if(s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\t\taddress: <%s>\n", host);
        }
        else if(addr->sa_family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats *stats = ifa->ifa_data;

            printf("\t\ttx_packets = %10u; rx_packets = %10u; "
                   "tx_bytes   = %10u; rx_bytes   = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);
}