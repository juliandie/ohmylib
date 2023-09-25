#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>


#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>

#define BUFFER_SIZE 4096

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const char *sa_family_strings[] = {
    "AF_UNSPEC",
    "AF_LOCAL",
    "AF_INET",
    "AF_AX25",
    "AF_IPX",
    "AF_APPLETALK",
    "AF_NETROM",
    "AF_BRIDGE",
    "AF_ATMPVC",
    "AF_X25",
    "AF_INET6",
    "AF_ROSE",
    "AF_DECnet",
    "AF_NETBEUI",
    "AF_SECURITY",
    "AF_KEY",
    "AF_NETLINK", // "AF_ROUTE",
    "AF_PACKET",
    "AF_ASH",
    "AF_ECONET",
    "AF_ATMSVC",
    "AF_RDS",
    "AF_SNA",
    "AF_IRDA",
    "AF_PPPOX",
    "AF_WANPIPE",
    "AF_LLC",
    "AF_IB",
    "AF_MPLS",
    "AF_CAN",
    "AF_TIPC",
    "AF_BLUETOOTH",
    "AF_IUCV",
    "AF_RXRPC",
    "AF_ISDN",
    "AF_PHONET",
    "AF_IEEE802154",
    "AF_CAIF",
    "AF_ALG",
    "AF_NFC",
    "AF_VSOCK",
    "AF_KCM",
    "AF_QIPCRTR",
    "AF_SMC",
};

static void print_netif(struct ifaddrs *ifa) {
    struct sockaddr *addr = ifa->ifa_addr;
    const char *sa = NULL;

    if(addr->sa_family < ARRAY_SIZE(sa_family_strings))
        sa = sa_family_strings[addr->sa_family];

    printf("%-8s %s (%d)\n", ifa->ifa_name, sa, addr->sa_family);
}

int getgatewayandiface() {
    int     received_bytes = 0, msg_len = 0, route_attribute_len = 0;
    int     sock = -1;
    uint32_t msgseq = 0;
    struct  nlmsghdr *nlh, *nlmsg;
    struct  rtmsg *route_entry;
    // This struct contain route attributes (route type)
    struct  rtattr *route_attribute;
    char    gateway_address[INET_ADDRSTRLEN], interface[IF_NAMESIZE];
    char    msgbuf[BUFFER_SIZE], buffer[BUFFER_SIZE];
    char *ptr = buffer;
    struct timeval tv;

    if((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    memset(msgbuf, 0, sizeof(msgbuf));
    memset(gateway_address, 0, sizeof(gateway_address));
    memset(interface, 0, sizeof(interface));
    memset(buffer, 0, sizeof(buffer));

    /* point the header and the msg structure pointers into the buffer */
    nlmsg = (struct nlmsghdr *)msgbuf;

    /* Fill in the nlmsg header*/
    nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlmsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
    nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlmsg->nlmsg_seq = msgseq++; // Sequence of the message packet.
    nlmsg->nlmsg_pid = getpid(); // PID of process sending the request.

    /* 1 Sec Timeout to avoid stall */
    tv.tv_sec = 1;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
    /* send msg */
    if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
        perror("send failed");
        return EXIT_FAILURE;
    }

    /* receive response */
    do {
        received_bytes = recv(sock, ptr, sizeof(buffer) - msg_len, 0);
        if(received_bytes < 0) {
            perror("Error in recv");
            return EXIT_FAILURE;
        }

        nlh = (struct nlmsghdr *)ptr;

        /* Check if the header is valid */
        if((NLMSG_OK(nlmsg, received_bytes) == 0) ||
           (nlmsg->nlmsg_type == NLMSG_ERROR)) {
            perror("Error in received packet");
            return EXIT_FAILURE;
        }

        /* If we received all data break */
        if(nlh->nlmsg_type == NLMSG_DONE)
            break;
        else {
            ptr += received_bytes;
            msg_len += received_bytes;
        }

        /* Break if its not a multi part message */
        if((nlmsg->nlmsg_flags & NLM_F_MULTI) == 0)
            break;
    } while((nlmsg->nlmsg_seq != msgseq) || (nlmsg->nlmsg_pid != (uint32_t)getpid()));

    /* parse response */
    for(; NLMSG_OK(nlh, received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes)) {
        /* Get the route data */
        route_entry = (struct rtmsg *)NLMSG_DATA(nlh);

        /* We are just interested in main routing table */
        if(route_entry->rtm_table != RT_TABLE_MAIN)
            continue;

        route_attribute = (struct rtattr *)RTM_RTA(route_entry);
        route_attribute_len = RTM_PAYLOAD(nlh);

        /* Loop through all attributes */
        for(; RTA_OK(route_attribute, route_attribute_len);
            route_attribute = RTA_NEXT(route_attribute, route_attribute_len)) {
            switch(route_attribute->rta_type) {
            case RTA_OIF:
                if_indextoname(*(int *)RTA_DATA(route_attribute), interface);
                break;
            case RTA_GATEWAY:
                inet_ntop(AF_INET, RTA_DATA(route_attribute),
                          gateway_address, sizeof(gateway_address));
                break;
            default:
                break;
            }
        }

        if((*gateway_address) && (*interface)) {
            fprintf(stdout, "Gateway %s for interface %s\n", gateway_address, interface);
            break;
        }
    }

    close(sock);

    return 0;
}

int main(/*int argc, char *argv[]*/) {
    struct ifaddrs *ifaddr;
    struct sockaddr *addr;
    int s;
    char host[NI_MAXHOST];

    if(getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    getgatewayandiface();
    /* Walk through linked list, maintaining head pointer so we can free list later */
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL;
        ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == NULL)
            continue;

        addr = ifa->ifa_addr;

        /* For an AF_INET* interface address, display the address */

        switch(addr->sa_family) {
        case AF_INET:
            print_netif(ifa);
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
            break;
        case AF_INET6:
#if 0
            print_netif(ifa);
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
#endif
            break;
        case AF_PACKET:
        {
#if 1
            struct rtnl_link_stats *stats = ifa->ifa_data;

            if(!stats)
                continue;

            print_netif(ifa);

            printf("\t\ttx_packets = %10u; "
                   "rx_packets = %10u; "
                   "tx_bytes   = %10u; "
                   "rx_bytes   = %10u\n",
                   stats->tx_packets,
                   stats->rx_packets,
                   stats->tx_bytes,
                   stats->rx_bytes);
#endif
        } break;
        default:
            print_netif(ifa);
            printf("\t\tdunno\n");
            break;
        }
    }

    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);
}