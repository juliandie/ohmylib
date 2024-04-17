#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <lib_queue.h>

#define STATIC (1)

struct elemen_s {
    int idx;
    uint64_t val;
    void *p;
};

#define ELEMENT_COUNT (1024)
static struct elemen_s elements[ELEMENT_COUNT];
static struct elemen_s my_elements[ELEMENT_COUNT];

static void *elements_reader(void *arg) {
    struct lib_queue_s *q = (struct lib_queue_s *)arg;
    uint64_t tid = pthread_self();

    printf("start thread %lu\n", tid);
    for(;;) {
#if defined(STATIC) && STATIC == 1
        struct elemen_s e;
        if(lib_queue_recv(q, &e, 100) == 0) {
            printf("thread %ld got %d\n", tid, e.idx);
#else
        struct elemen_s *e;
        if(lib_queue_recv(q, &e, 100) == 0) {
            printf("thread %ld got %d\n", tid, e->idx);
#endif
        }
        else {
            //printf("thread %ld got nothing\n", tid);
        }

        usleep(100);
    }
    pthread_exit(NULL);
}

#define THREADS (5)
int main() {
    pthread_t th[THREADS];
    struct lib_queue_s *q;
    int ret;
#if defined(STATIC) && STATIC == 1
    q = lib_queue_create_static(elements,
                                sizeof(elements),
                                ELEMENT_COUNT,
                                sizeof(elements[0]));
#else
    q = lib_queue_create(ELEMENT_COUNT, sizeof(&elements[0]));
#endif

    for(int i = 0; i < THREADS; i++) {
        ret = pthread_create(&th[i], NULL, &elements_reader, q);
        if(ret != 0) {
            return -1;
        }
    }
    usleep(100);
    for(int i = 0; i < 16; i++) {
        struct elemen_s *e = &my_elements[i];
        e->idx = i;
        //printf("queue (%d)\n", my_elements[i].idx);
#if defined(STATIC) && STATIC == 1
        lib_queue_send(q, e, 1000);
#else
        lib_queue_send(q, &e, 1000);
#endif
    }
    usleep(100);

    for(int i = 0; i < THREADS; i++) {
        printf("cancel(%d)\n", i);
        pthread_cancel(th[i]);
#if 0
    }
    for(int i = 0; i < THREADS; i++) {
        pthread_join(th[i], NULL);
    }
#else
        pthread_join(th[i], NULL);
}
#endif

    return 0;
}
