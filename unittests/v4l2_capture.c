#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include <lib_v4l2.h>
#include <lib_log.h>

struct capture_s {
    pthread_mutex_t lock;
    pthread_t thread;

    char *videodev;

    int videofd;
    struct v4l2_pix_format format;
    void **buffer;
    uint32_t n_buffer;

    uint32_t t_frames; // total frames received
    uint32_t n_frames; // frames since capture start

    int show_fps;
    int restart;
    int capture;
    int opened;
    int show;
    int run;
};

static int capture_close(struct capture_s *cap) {
    if(!cap) {
        errno = EINVAL;
        return -1;
    }

    v4l_munmap_buffer(cap->videofd, &cap->buffer, cap->n_buffer);
    v4l_close(cap->videofd);
    cap->videofd = -1;
    cap->n_buffer = 0;
    cap->opened = 0;

    return 0;
}

static int capture_open(struct capture_s *cap) {
    int ret;

    if(!cap) {
        errno = EINVAL;
        return -1;
    }

    cap->videofd = v4l_open(cap->videodev);
    if(cap->videofd < 0)
        goto err;

    if(v4l_get_format(cap->videofd, &cap->format) < 0) {
        LIB_LOG_ERR("couldn't get format");
    }

    LIB_LOG_INFO("%ux%u (0x%08x, %.4s)",
                 cap->format.width, 
                 cap->format.height,
                 cap->format.pixelformat, 
                 (char *)&cap->format.pixelformat);

    ret = v4l_mmap_buffer(cap->videofd, &cap->buffer, 4);
    if(ret < 0)
        goto err_close;

    cap->n_buffer = (uint32_t)ret;

    if(v4l_start(cap->videofd, cap->n_buffer) < 0)
        goto err_free;

    cap->opened = 1;

    return 0;

err_free:
    v4l_munmap_buffer(cap->videofd, &cap->buffer, cap->n_buffer);
err_close:
    v4l_close(cap->videofd);
err:
    return -1;
}

static int capture_store(struct capture_s *cap, const void *buf, size_t count) {
    char fn[256];
    int fd, ret;
    size_t size = 0;

    if(snprintf(fn, sizeof(fn), "frame-%06d.bin", cap->t_frames) < 0)
        goto err;

    fd = open(fn, O_RDWR | O_CREAT);
    if(fd < 0)
        goto err;

    while(size < count) {
        ret = write(fd, buf + size, count - size);
        if(ret < 0)
            goto err_close;

        size += (size_t)ret;
    }

    close(fd);

    return 0;

err_close:
    close(fd);
err:
    return -1;
}

static void *capture_thread(void *arg) {
    struct capture_s *cap = (struct capture_s *)arg;
    struct timespec tp[2];
    int ret, buf_idx;
    float elapsed;
    void *buf;

    if(!cap)
        pthread_exit(NULL);

    cap->run = 1;

    while(cap->run) {
        if(!cap->opened) {
            if(capture_open(cap) < 0) {
                usleep(1000);
                continue;
            }
            clock_gettime(CLOCK_REALTIME, &tp[0]);
        }

        if(cap->restart) {
            capture_close(cap);
            continue;
        }

        ret = v4l_poll(cap->videofd, 1000);
        if(ret < 0) {
            capture_close(cap);
            continue;
        }

        buf_idx = v4l_dqbuf(cap->videofd);
        if(buf_idx < 0) {
            switch(errno) {
            case EAGAIN: //continue;
            case EIO: /* Could ignore EIO, see spec. */
            default: /* fall through */
                capture_close(cap);
                continue;
            }
        }

        if((size_t)buf_idx > cap->n_buffer)
            continue;

        cap->t_frames++;
        cap->n_frames++;

        clock_gettime(CLOCK_REALTIME, &tp[1]);
        elapsed = ((tp[1].tv_sec * 1000000000ul) + tp[1].tv_nsec) -
                  ((tp[0].tv_sec * 1000000000ul) + tp[0].tv_nsec);
        memcpy(&tp[0], &tp[1], sizeof(struct timespec));

        buf = cap->buffer[buf_idx];

        if(cap->show_fps) {
            printf("[f %d/%d] [%.1f FPS]", cap->n_frames, cap->t_frames,
                   (1.0f / (elapsed / 1000000000)));
        }

        if(cap->capture) {
            printf("[c] ");
            capture_store(cap, buf, 1024);
        }

        if(v4l_qbuf(cap->videofd, buf_idx) < 0) {
            capture_close(cap);
            continue;
        }
        printf("\r");
        fflush(stdout);
    }
    printf("\n");
    pthread_exit(NULL);
}

static void getchar_once() {
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
    //turn off canonical mode
    ttystate.c_lflag &= ~ICANON;
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

static void usage() {
    printf("help\n");
}

int main(int argc, char **argv) {
    struct capture_s cap;
    int ret;

    memset(&cap, 0, sizeof(struct capture_s));
    cap.videodev = "/dev/video0";
    
    ret = pthread_mutex_init(&cap.lock, NULL);
    if(ret < 0) {
        LIB_LOG_ERR("failed to init mutex");
        return -1;
    }

    ret = pthread_create(&cap.thread, NULL, &capture_thread, (void *)&cap);
    if(ret != 0)
        return -1;

    getchar_once();
    usage();
    
    for(;;) {
        int c = fgetc(stdin); // don't use getchar tho!

        switch(c) {
        case 'q': goto done;
        default: break;
        }
        usleep(1000);
    }
done:
    cap.run = 0;
    pthread_join(cap.thread, NULL);

    return 0;
}