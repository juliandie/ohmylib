#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <lib_sock.h>
#include <lib_log.h>

#define PORT (8081)

static volatile int run = 1;

static void sig_reset(int signum) {
	signal(signum, SIG_DFL);
}

static int sig_register(int signum, void (*handler)(int)) {
	struct sigaction act = {
		.sa_flags = SA_RESTART,
		.sa_handler = handler,
	};
	sigemptyset(&act.sa_mask);

	return sigaction(signum, &act, NULL);
}

static void sig_hdl(int sig) {
	run = 0;
	sig_reset(sig);
}

static int server(int fd) {
	struct sockaddr_in src_addr;
	socklen_t addrlen;
	char buf[1500];
	int ret;

	while(run) {
        ret = lib_poll(fd, 1000);
        if(ret < 0)
            return -1;

        if(ret == 0)
            continue;

		ret = recvfrom(fd, buf, sizeof(buf), 0,
					   (struct sockaddr *)&src_addr, &addrlen);
		if(ret < 0)
			return -1;

		if(ret == 0)
			continue;
		
		lib_hexdump(buf, ret, "From %s (%dB)", inet_ntoa(src_addr.sin_addr), ret);

		ret = lib_sendto(fd, buf, ret, 0, (struct sockaddr *)&src_addr, addrlen);

		if(ret < 0)
			return -1;

		if(ret == 0)
			continue;
	}

	return 0;
}

int main(/* int argc, char **argv */) {
	__be16 port;
	int fd;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		goto err;

	if(lib_bind4(fd, INADDR_ANY, htons(PORT)) < 0)
		goto err_close;

    if(fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
        LIB_LOG_ERR("F_SETFL: %s", strerror(errno));

	if(lib_sock_mtu_discover_do(fd) < 0)
		LIB_LOG_ERR("IP_MTU_DISCOVER: %s", strerror(errno));

	if(lib_port(fd, &port) < 0) {
		goto err_close;
	}

	LIB_LOG_INFO("port %d", htons(port));

	if(server(fd) < 0)
		goto err_close;

	close(fd);
	return 0;

err_close:
	close(fd);
err:
	return -1;
}

#if 0
static int my_listener(udp_sock*udp, lib_netpkt *frame) {
	if (udp == NULL || frame == NULL) {
		LIB_LOG_ERR("something went wrong");
		run = 0;
		return -1;
	}

	udp_dump_rx_frame(frame);

	lib_buffer_puta(&frame->tx, "echo -- ", 8);
	lib_buffer_puta(&frame->tx, frame->rx.data, frame->rx.len);

	if(frame->tx.data[frame->tx.len - 1] != '\n')
		lib_buffer_puta(&frame->tx, "\n", 1);

	udp_send(udp, frame);
	return 0;
}

int main(int argc, char **argv) {
	lib_netpkt frame;
	struct udp_sock udp;
	int ret;

	sig_register(SIGINT, sig_hdl);

	if(udp_open(&udp, htonl(INADDR_ANY), htons(PORT)) < 0) {
		return -1;
	}

	if(lib_net_init_netpkt_async(&frame, 8, 32)) {
		return -1;
	}

	if (lib_net_enable_pktinfo(udp.sd) < 0)
		LIB_LOG_ERR("set IP_PKTINFO failed");

	if (lib_net_enable_broadcast(udp.sd) < 0)
		LIB_LOG_ERR("set SO_BROADCAST failed");

	if (lib_net_disable_fragment(udp.sd) < 0)
		LIB_LOG_WARNING("set SOL_IP failed");

	LIB_LOG_INFO("listening on %d", udp.port);
	udp.listener = my_listener;

	while (run) {
		ret = udp_poll(&udp, -1);
		switch (ret) {
		case 0:
			break;
		case 1:
			udp_recv(&udp, &frame);
			break;
		case -1:
		default:
			run = 0;
			break;
		}
	}

	lib_udp_close(&udp);

	return 0;
}
#endif