#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <lib_queue.h>

#define STATIC (1)

struct elemen_s {
    int idx;
    uint64_t val;
    void *p;
};

#define ELEMENT_COUNT (1024)
#define THREADS_COUNT (5)

static struct elemen_s *elements[ELEMENT_COUNT];

static void *elements_reader(void *arg) {
    struct lib_queue_s *q = (struct lib_queue_s *)arg;
    uint64_t tid = pthread_self();

    printf("start thread %lu\n", tid);
    for(;;) {
#if defined(STATIC) && STATIC == 1
        struct elemen_s *e;
        if(lib_queue_recv(q, &e, 100) == 0) {
            printf("thread %ld got %d\n", tid, e->idx);
#else
        struct elemen_s *e;
        if(lib_queue_recv(q, &e, 100) == 0) {
            printf("thread %ld got %d\n", tid, e->idx);
#endif
            free(e);
        }
        else {
            //printf("thread %ld got nothing\n", tid);
        }

        usleep(100);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t th[THREADS_COUNT];
    struct lib_queue_s *q;
    int ret;
#if defined(STATIC) && STATIC == 1
    q = lib_queue_create_static(elements,
                                sizeof(elements),
                                ELEMENT_COUNT,
                                sizeof(elements[0]));
#else
    q = lib_queue_create(ELEMENT_COUNT, sizeof(elements[0]));
#endif

    for(int i = 0; i < THREADS_COUNT; i++) {
        ret = pthread_create(&th[i], NULL, &elements_reader, q);
        if(ret != 0) {
            return -1;
        }
    }

    for(int i = 0; i < (ELEMENT_COUNT * 2); i++) {
        struct elemen_s *e;
        e = calloc(1, sizeof(*e));
        e->idx = i;
        e->p = q;
        if(lib_queue_send(q, &e, 1000) < 0) {
            printf("failed to push %d\n", i);
        }
    }
    while(lib_queue_empty_wait(q, 1000)) {
        //printf("wait for q-empty\n");
}
    printf("cleanup (%d)\n", q->avail);

    for(int i = 0; i < THREADS_COUNT; i++) {
        printf("cancel(%d)\n", i);
        pthread_cancel(th[i]);
        pthread_join(th[i], NULL);
}

    return 0;
}
