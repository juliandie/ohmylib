#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <queue.h>

#define Q_CNT (4)
#define Q_STATIC (0)
#define _aligned(x) __attribute__((aligned(x)))

struct some_struct_s {
    int val;
};

#if defined(Q_STATIC) && Q_STATIC==1
static char _aligned(64) buf[sizeof(struct some_struct_s *) * Q_CNT];
#endif

static void q_stat(struct queue_s *q) {
    fprintf(stdout, "q empty: %d, full: %d, free: %d\n",
            queue_is_empty(q),
            queue_is_full(q),
            queue_space_available(q));
}

int main() {
    struct queue_s *q;
    int ret;

#if defined(Q_STATIC) && Q_STATIC==1
    q = queue_create_static(buf, sizeof(buf), Q_CNT, sizeof(struct some_struct_s *));
#else
    q = queue_create(Q_CNT, sizeof(struct some_struct_s *));
#endif
    if(!q) {
        fprintf(stderr, "failed to create queue: %s\n", strerror(errno));
        return -1;
    }

    q_stat(q);
    for(int i = 0; ; i++) {
        struct some_struct_s *s;
        if(i == 3) {
            q_stat(q);
        }
        s = calloc(1, sizeof(*s));
        if(!s) {
            goto err_clear;
        }
        s->val = i + 1;
        ret = queue_send(q, &s, 1000);
        if(ret < 0) {
            fprintf(stderr, "failed to queue data: %s\n", strerror(errno));
            free(s);
            goto send_done;
        }
    };
send_done:
    q_stat(q);

    for(int i = 0; ; i++) {
        struct some_struct_s *s = NULL;
        ret = queue_recv(q, &s, 1000);
        if(ret < 0) {
            fprintf(stderr, "failed to dequeue data: %s\n", strerror(errno));
            goto recv_done;
        }
        printf("%d;\n", s->val);
        free(s);
    }
recv_done:
    q_stat(q);

err_clear:
    queue_destroy(q);
    return 0;
}