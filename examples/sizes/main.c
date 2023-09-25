#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>

#if 0
#define _packed __attribute__((packed))
#else
#define _packed
#endif

struct _packed jo_discovery {
	union {
		struct { // request
			uint16_t port;
		} req;
		struct { // response
			uint32_t ipv4;
			uint32_t netmask;
			uint8_t mac[6];
		} res;
	};
};

struct _packed jo_cmd {
	uint16_t magic;
	uint16_t cmd;
	uint16_t len; /**< len of payload */
	uint16_t seq;
	uint8_t payload[];
};

int main() {
	struct jo_discovery a;
    printf("(struct jo_discovery): %luB\n", sizeof(struct jo_discovery));
    printf("(struct jo_discovery)->req: %luB\n", sizeof(a.req));
    printf("(struct jo_discovery)->res: %luB\n", sizeof(a.res));
	
    printf("(struct jo_cmd): %luB\n", sizeof(struct jo_cmd));
    return 0;
}