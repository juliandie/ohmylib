#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include <queue.h>

static struct queue_s *queue_setup(void *p, uint32_t nmemb, uint32_t size) {
    struct queue_s *q;

    q = calloc(1, sizeof(struct queue_s));
    if(!q) {
        return NULL;
    }

    (void)pthread_mutex_init(&q->lock, NULL);
    (void)pthread_cond_init(&q->cond_write, NULL);
    (void)pthread_cond_init(&q->cond_read, NULL);
    (void)pthread_cond_init(&q->cond_empty, NULL);
    q->p_head = q->p_write = q->p_read = p;
    q->p_tail = (uint8_t *)p + ((nmemb - 1) * size);
    q->nmemb = nmemb;
    q->size = size;
    return q;
}

struct queue_s *queue_create_static(void *p, size_t count,
    size_t nmemb, size_t size) {
    struct queue_s *q;

    if(!nmemb || !size || ((nmemb * size) < count)) {
        errno = EINVAL;
        return NULL;
    }

    q = queue_setup(p, nmemb, size);
    if(q) {
        q->statically_allocated = 1;
    }

    return q;
}

struct queue_s *queue_create(size_t nmemb, size_t size) {
    struct queue_s *q;
    void *p;

    if(!nmemb || !size) {
        errno = EINVAL;
        return NULL;
    }

    p = calloc(1, nmemb * size);
    if(!p) {
        return NULL;
    }

    q = queue_setup(p, nmemb, size);
    if(!q) {
        free(p);
    }

    return q;
}

void queue_destroy(struct queue_s *q) {
    if(!q) {
        return;
    }

    if(!q->statically_allocated) {
        free(q->p_head);
    }
    free(q);
}

static int queue_is_empty_locked(struct queue_s *q) {
    return !q->avail;
}

int queue_is_empty(struct queue_s *q) {
    int ret = 0;
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    (void)pthread_mutex_lock(&q->lock);
    ret = queue_is_empty_locked(q);
    (void)pthread_mutex_unlock(&q->lock);
    return ret;
}

static int queue_is_full_locked(struct queue_s *q) {
    return (q->nmemb == q->avail);
}

int queue_is_full(struct queue_s *q) {
    int ret = 0;
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    (void)pthread_mutex_lock(&q->lock);
    ret = queue_is_full_locked(q);
    (void)pthread_mutex_unlock(&q->lock);
    return ret;
}

static int queue_space_available_locked(struct queue_s *q) {
    return (q->nmemb - q->avail);
}

int queue_space_available(struct queue_s *q) {
    int ret;
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    (void)pthread_mutex_lock(&q->lock);
    ret = queue_space_available_locked(q);
    (void)pthread_mutex_unlock(&q->lock);
    return ret;
}

static void  queue_ts_add(struct timespec *ts, int timeout) {
    ts->tv_nsec += ((timeout % 1000) * (1000000l));
    if(ts->tv_nsec > 999999999l) {
        ts->tv_nsec -= 999999999l;
        ts->tv_sec += (timeout / 1000) + 1;
    }
    else {
        ts->tv_sec += (timeout / 1000);
    }
}

static int queue_send_wait(struct queue_s *q, int timeout) {
    struct timespec ts;
    int oldstate;
    int ret = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    if(timeout) {
        queue_ts_add(&ts, timeout);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    while(queue_is_full_locked(q) && ret == 0) {
        ret = pthread_cond_timedwait(&q->cond_write, &q->lock, &ts);
    }
    pthread_setcancelstate(oldstate, NULL);
    return queue_is_full_locked(q);
}

int queue_send(struct queue_s *q, void *p, int timeout) {
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    (void)pthread_mutex_lock(&q->lock);
    if(queue_is_full_locked(q) && queue_send_wait(q, timeout)) {
        (void)pthread_mutex_unlock(&q->lock);
        errno = ENOSPC;
        return -1;
    }
    memcpy(q->p_write, p, q->size);
    if(q->p_write == q->p_tail) {
        q->p_write = q->p_head;
    }
    else {
        q->p_write = (uint8_t *)q->p_write + q->size;
    }
    q->avail++;
    pthread_cond_broadcast(&q->cond_read);
    (void)pthread_mutex_unlock(&q->lock);
    return 0;
}

static int queue_recv_wait(struct queue_s *q, int timeout) {
    struct timespec ts;
    int oldstate;
    int ret = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    if(timeout) {
        queue_ts_add(&ts, timeout);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    while(queue_is_empty_locked(q) && ret == 0) {
        ret = pthread_cond_timedwait(&q->cond_read, &q->lock, &ts);
    }
    pthread_setcancelstate(oldstate, NULL);
    return queue_is_empty_locked(q);
}

int queue_recv(struct queue_s *q, void *p, int timeout) {
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    (void)pthread_mutex_lock(&q->lock);
    if(queue_is_empty_locked(q) && queue_recv_wait(q, timeout)) {
        pthread_cond_broadcast(&q->cond_empty);
        (void)pthread_mutex_unlock(&q->lock);
        errno = EAGAIN;
        return -1;
    }
    memcpy(p, q->p_read, q->size);
    if(q->p_read == q->p_tail) {
        q->p_read = q->p_head;
    }
    else {
        q->p_read = (uint8_t *)q->p_read + q->size;
    }
    q->avail--;
    pthread_cond_broadcast(&q->cond_write);
    (void)pthread_mutex_unlock(&q->lock);
    return 0;
}

int queue_empty_wait(struct queue_s *q, int timeout) {
    if(!q) {
        errno = EINVAL;
        return -1;
    }
    struct timespec ts;
    int oldstate;
    int ret = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    if(timeout) {
        queue_ts_add(&ts, timeout);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    while(!queue_is_empty_locked(q) && ret == 0) {
        ret = pthread_cond_timedwait(&q->cond_empty, &q->lock, &ts);
    }
    pthread_setcancelstate(oldstate, NULL);
    (void)pthread_mutex_unlock(&q->lock);
    return !queue_is_empty_locked(q);
}