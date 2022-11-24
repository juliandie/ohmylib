
#include <lib_raw.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <linux/types.h>

#include <arpa/inet.h>

#include <lib_log.h>
#include <lib_buffer.h>
#include <lib_netif.h>
#include <lib_udp.h>
#include <lib_netif.h>

#define IPFRAG_OFF_DONT (IPTOS_PREC_IMMEDIATE) // 0x40
#define IPFRAG_OFF_MORE (IPTOS_PREC_PRIORITY) // 0x20

#if 0
struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8     ihl : 4,
		version : 4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8 version : 4,
		ihl : 4;
#endif
	__u8    tos;
	__be16  tot_len;
	__be16  id;
	__be16  frag_off;
	__u8    ttl;
	__u8    protocol;
	__sum16 check;
	__be32  saddr;
	__be32  daddr;
	/*The options start here. */
};

struct udphdr {
	__be16 source;
	__be16 dest;
	__be16 len;
	__sum16 check;
};

struct icmphdr {
	__u8      type;
	__u8      code;
	__sum16   checksum;
	union {
		struct {
			__be16  id;
			__be16  sequence;
		} echo;
		__be32  gateway;
		struct {
			__be16  __unused;
			__be16  mtu;
		} frag;
		__u8    reserved[4];
	} un;
};

struct dhcphdr {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	char chaddr[16];
	char sname[64];
	char file[128];
	char options[312];
};

#endif

static struct iphdr* raw_get_iphdr(lib_buffer* buf) {
	struct iphdr* ip;

	if (buf->len - buf->offs < (int)sizeof(struct iphdr)) {
		return NULL;
	}
	ip = (struct iphdr*)(buf->data + buf->offs);

	if (ip->version != IPVERSION) {
		//LIB_LOG_DEBUG("ipv:%d", ip->version);
		return NULL;
	}

	if (ip->ihl != 0x5 /*sizeof(struct iphdr) >> 2*/) {
		return NULL;
	}

	buf->offs += sizeof(struct iphdr);
	return ip;
}

#if 0
static int lib_raw_set_iphdr(lib_buffer* buf, struct iphdr* ip) {
	memcpy(&buf->data[buf->offs], ip, sizeof(struct iphdr));
	lib_buffer_append(buf, sizeof(struct iphdr));
	return sizeof(struct iphdr);
}
#endif

static struct udphdr* raw_get_udphdr(lib_buffer* buf) {
	struct udphdr* udp;

	if (buf->len - buf->offs < (int)sizeof(struct udphdr))
		return NULL;

	udp = (struct udphdr*)(buf->data + buf->offs);
	buf->offs += sizeof(struct udphdr);

	return udp;
}

#if 0
static int raw_set_udphdr(lib_buffer* buf, struct udphdr* udp) {
	memcpy(&buf->data[buf->offs], udp, sizeof(struct udphdr));
	lib_buffer_append(buf, sizeof(struct udphdr));
	return sizeof(struct udphdr);
}
#endif

static struct icmphdr* raw_get_icmphdr(lib_buffer* buf) {
	struct icmphdr* icmp;

	if (buf->len - buf->offs < (int)sizeof(struct icmphdr))
		return NULL;

	icmp = (struct icmphdr*)(buf->data + buf->offs);
	buf->offs += sizeof(struct icmphdr);;

	return icmp;
}

#if 0
static void raw_set_icmphdr(lib_buffer* buf, struct icmphdr* icmp) {
	memcpy(&buf->data[buf->offs], icmp, sizeof(struct icmphdr));
	lib_buffer_append(buf, sizeof(struct icmphdr));
	return sizeof(struct icmphdr);
}
#endif

raw_sock* raw_create(const char* ifname) {
	raw_sock* raw;

	raw = malloc(sizeof(raw_sock));
	if (!raw)
		return NULL;

	memset(raw, 0, sizeof(raw_sock));
	if (raw_open(raw, ifname)) {
		goto err;
	}

	return raw;

err:
	raw_destroy(raw);
	return NULL;
}

int raw_open(raw_sock* raw, const char* ifname) {
	struct sockaddr_ll sa_raw;

	raw->sysidx = lib_netif_idx(ifname);
	if (raw->sysidx < 0) {
		//LIB_LOG_ERR("sysif idx");
		goto err;
	}

	raw->sd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if (raw->sd < 0) {
		//LIB_LOG_ERR("socket");
		goto err;
	}

	sa_raw.sll_family = AF_PACKET;
	sa_raw.sll_protocol = htons(ETH_P_IP);
	sa_raw.sll_ifindex = raw->sysidx;

	if (bind(raw->sd, (struct sockaddr*)(&sa_raw), sizeof(sa_raw))) {
		//LIB_LOG_ERR("bind");
		goto err;
	}

	if (lib_buffer_alloc(&raw->frame.rx, 1500) < 0) { // Allocate a typical mtu
		//LIB_LOG_WARNING("alloc rx frame");
		goto err;
	}

	if (lib_buffer_alloc(&raw->frame.tx, 1500) < 0) { // Allocate a typical mtu
		//LIB_LOG_WARNING("alloc tx frame");
		goto err;
	}

	raw->ifname = strdup(ifname);

	return 0;

err:
	raw_close(raw);
	memset(raw, 0, sizeof(raw_sock));
	return -1;
}

void raw_close(raw_sock* raw) {
	if (!raw)
		return;

	if (raw->ifname)
		free(raw->ifname);

	if (raw->sd >= 0)
		close(raw->sd);

	raw->sd = -1;
}

void raw_destroy(raw_sock* raw) {
	if (raw == NULL)
		return;

	raw_close(raw);
	free(raw);
}

int raw_poll(raw_sock* raw, int timeout) {
	struct pollfd fds;
	int ret;

	fds.fd = raw->sd;
	fds.events = (POLLIN | POLLPRI);
	ret = poll(&fds, 1, timeout);

	if (ret < 0) { // error
		LIB_LOG_ERR("poll");
		return -1;
	}

	if (ret > 0 && (fds.revents & POLLIN || fds.revents & POLLPRI)) {
		return 1;
	}

	return 0; // timeout
}

static int raw_parse_icmp(raw_sock* raw) {
	lib_netpkt* frame = &raw->frame;

	struct icmphdr* raw_icmp;
	struct udphdr* raw_udp;
	struct iphdr* raw_ip;

	raw_icmp = raw_get_icmphdr(&frame->rx);
	if (raw_icmp == NULL)
		return -1;

	// Check type (only interested in "destination unreachable")
	if (raw_icmp->type != ICMP_DEST_UNREACH)
		return -1;

	// Check code (ignore "soft" errors per RFC 1122)
	if ((raw_icmp->code == ICMP_NET_UNREACH) ||
		(raw_icmp->code == ICMP_HOST_UNREACH) ||
		(raw_icmp->code == ICMP_SR_FAILED))
		return -1;

	raw_ip = raw_get_iphdr(&frame->rx);
	if (raw_ip == NULL)
		return -1;

	// Check IP header of error-causing packet (only interested in UDP)
	if (raw_ip->protocol != IPPROTO_UDP)
		return -1;

	// Decompose UDP of error-causing packet and check ports (source must be equal to local source port)
	raw_udp = raw_get_udphdr(&frame->rx);
	if (raw_udp->source != raw_udp->dest)
		return -1;

	frame->addr.sin_port = raw_udp->source;
	frame->addr.sin_port = raw_udp->dest;
	frame->raw = 1;

	return 0;
}

#if 0
static int raw_parse_udp(raw_sock* raw) {
#if 0 // is this needed? we're already listening to an udp socket
	struct udphdr* raw_udp;

	// Check L2 packet type - we are only interested in broadcasts here
	if (addr.sll_pkttype != PACKET_BROADCAST)
		goto done;

#if 0 // what is a wrong bcast?
	// Check if the socket is set to accept wrong subnet broadcasts
	if (!(s->accept_wrong_bcast))
		goto done;
#endif

	// Check IP header flags, UDP protocol, destination address = broadcast
	if (ntohs(ip->frag_off) & IPFRAG_OFF_MORE)
		goto done;

	if (ip->daddr != INADDR_BROADCAST)
		goto done;

#if 0
	/* On kernels < 2.6.27.9 those packets are delivered to the standard UDP
	 * socket when not using a link-local address. This has to be checked here
	 * so that those packets are not handled twice.
	 */
	if ((!kernel_at_least_2_6_27_9) && ((s->saddr.sin_addr.s_addr & 0x0000FFFF) != 0xFEA9))
		goto done;
#endif
	if (raw->udp == NULL)
		goto done;

	raw_udp = raw_des_udp(&frame.rx);
	if (raw_udp == NULL || ntohs(raw_udp->dst_port) != raw->udp->port)
		goto done;

	// Check packet length in UDP header vs. actual received
	if (ntohl(raw_udp->datagram_len) > (uint32_t)(rx->len - rx->offs))
		goto done;

	frame.brd = 1;
	frame.raw = 1;

	frame.from.sin_addr.s_addr = ip->saddr;
	frame.from.sin_port = raw_udp->source;

	frame.addr.sin_addr.s_addr = ip->daddr;
	frame.addr.sin_port = raw_udp->dest;

	if (raw->udp->listener != NULL) {
		if (lib_buffer_alloc(&frame.tx, LIB_MAX_SENDBUF)) {
			LIB_LOG_ERR("malloc");
			return -1;
		}
		frame.tx.offs = (LIB_IPHDR_SIZE + LIB_UDPHDR_SIZE);

		ret = raw->udp->listener(raw->udp, &frame);
	}
#else
	(void)raw;
#endif
	return 0;
}
#endif

int raw_recv(raw_sock* raw) {
	lib_netpkt* frame;
	struct iphdr* ip;
	int ret;

	if (!raw) {
		errno = EINVAL;
		return -1;
	}

	frame = &raw->frame;

	struct sockaddr_ll addr;
	socklen_t addrlen = sizeof(struct sockaddr_ll);

	lib_buffer_clear(&frame->rx);
	ret = recvfrom(raw->sd, frame->rx.data, frame->rx.size, MSG_DONTWAIT | MSG_TRUNC,
		(struct sockaddr*)(&addr), &addrlen);

	if (ret < 0) {
		return -1;
	}

	if ((size_t)ret > frame->rx.size) {
		return 0;
	}

	frame->rx.len = ret;

	ip = raw_get_iphdr(&frame->rx);
	if (!ip)
		errno = EPROTO;
	return -1;

	if (ntohs(ip->frag_off) & IPFRAG_OFF_DONT) {
		return 0;
	}

	frame->addr.sin_addr.s_addr = ip->saddr;
	frame->addr.sin_addr.s_addr = ip->daddr;

	switch (ip->protocol) {
	case IPPROTO_ICMP: ret = raw_parse_icmp(raw); break;
	case IPPROTO_TCP: ret = /* raw_parse_tcp(raw); */ 0; break;
	case IPPROTO_UDP: ret = /* raw_parse_udp(raw); */ 0; break;
	default: ret = -1; break;
	}

	if (!ret && raw->listener != NULL) {
		return raw->listener(raw, frame);
	}

	return ret;
}

int raw_udp_send(raw_sock* raw, lib_netpkt* frame) {
	struct sockaddr_ll dest_addr;
	struct iphdr* ip;
	struct udphdr* udp;
	int ret;

	udp = (struct udphdr*)&frame->tx.data[sizeof(struct iphdr)];
	udp->source = htons(frame->addr.sin_port);
	udp->dest = htons(frame->addr.sin_port);
	udp->len = htons(frame->tx.len);
	frame->tx.len += sizeof(struct udphdr);

	ip = (struct iphdr*)&frame->tx.data[0];
	ip->version = 0x4;
	ip->ihl = 0x5;
	ip->tot_len = htons(frame->tx.len);
	ip->id = htons(raw->ip_id++);
	ip->frag_off = htons(IPFRAG_OFF_DONT);
	ip->ttl = 128;
	ip->protocol = IPPROTO_UDP;
	ip->saddr = frame->addr.sin_addr.s_addr;
	ip->daddr = INADDR_BROADCAST;
	frame->tx.len += sizeof(struct iphdr);

	dest_addr.sll_family = AF_PACKET;
	dest_addr.sll_protocol = htons(ETH_P_IP);
	dest_addr.sll_ifindex = raw->sysidx;
	dest_addr.sll_hatype = 0;
	dest_addr.sll_pkttype = 0;
	dest_addr.sll_halen = ETH_ALEN;
	memset(&(dest_addr.sll_addr[0]), 0xFF, ETH_ALEN);

	ret = sendto(raw->sd, frame->tx.data, frame->tx.len, 0, (struct sockaddr*)(&dest_addr), sizeof(dest_addr));
	if (ret < 0)
		return -1;

	return 0;
}
