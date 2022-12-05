#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h> // sem_t
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <lib_log.h>

#include <lib_v4l2.h>

static int xioctl(int fd, int req, void *arg) {
    return ioctl(fd, req, arg);
}

int v4l_close(int fd) {
    return close(fd);
}

int v4l_open(const char *videodev) {
    struct v4l2_capability cap;
    struct stat st;
    int fd;

    if(stat(videodev, &st) < 0)
        return -1;

    if(!S_ISCHR(st.st_mode)) // is no character device
        return -1;

    fd = open(videodev, O_RDWR /* required */ | O_NONBLOCK, 0);
    if(fd < 0) {
        //LIB_LOG_ERR("open: %s", strerror(errno));
        return -1;
    }

    memset(&cap, 0, sizeof(struct v4l2_capability));
    if(xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        //LIB_LOG_ERR("VIDIOC_QUERYCAP: %s", strerror(errno));
        goto close_fd;
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        //LIB_LOG_WARNING("%s is no video capture device", videodev);
        goto close_fd;
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
        //LIB_LOG_WARNING("%s does not support streaming i/o", videodev);
        goto close_fd;
    }

    return fd;

close_fd:
    close(fd);
    return -1;
}

int v4l_set_format(int fd, struct v4l2_pix_format *format) {
    struct v4l2_format fmt;

    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    memcpy(&fmt.fmt, format, sizeof(struct v4l2_pix_format));

    if(xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        //LIB_LOG_ERR("VIDIOC_S_FMT");
        return -1;
    }

    return 0;
}

int v4l_get_format(int fd, struct v4l2_pix_format *format) {
    struct v4l2_format fmt;

    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(xioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
        //LIB_LOG_ERR("VIDIOC_G_FMT");
        return -1;
    }

    memcpy(format, &fmt.fmt, sizeof(struct v4l2_pix_format));

    return 0;
}

void v4l_munmap_buffer(int fd, void ***p, size_t n_buf) {
    void **ptr;

    if(!fd || !p || !n_buf)
        return;
    
    ptr = *p;

    for(int i = 0; i < (int)n_buf; i++) {
        struct v4l2_buffer buf;
        
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if(xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            //LIB_LOG_ERR("VIDIOC_QUERYBUF: %s", strerror(errno));
            continue;
        }

        if(munmap(ptr[i], buf.length) < 0)
            //LIB_LOG_ERR("munmap failed: %s", strerror(errno));
    }

    free(*p);
    *p = NULL;
}

int v4l_mmap_buffer(int fd, void ***p, size_t n_buf) {
    struct v4l2_requestbuffers req;
    void **ptr;
    size_t ret, i;

    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = n_buf;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(xioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        //LIB_LOG_ERR("VIDIOC_REQBUFS: %s", strerror(errno));
        return -1;
    }

    if(req.count < 2) {
        errno = ENOMEM;
        return -1;
    }

    ptr = calloc(req.count, sizeof(void *));
    if(!ptr) 
        return -1;

    for(i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if(xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            //LIB_LOG_ERR("VIDIOC_QUERYBUF");
            goto free_buffer;
        }

        ptr[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, 
                        MAP_SHARED, fd, buf.m.offset);

        if(MAP_FAILED == ptr[i]) {
            //LIB_LOG_ERR("mmap failed: %s", strerror(errno));
            goto free_buffer;
        }
    }
    ret = i;

    *p = ptr;
    return ret;

free_buffer:
    v4l_munmap_buffer(fd, &ptr, i);
    return -1;
}

int v4l_stop(int fd) {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(xioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        LIB_LOG_ERR("VIDIOC_STREAMOFF");
        return -1;
    }
    return 0;
}

int v4l_start(int fd, size_t n_buf) {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    size_t i;

    for(i = 0; i < n_buf; i++) {
        if(v4l_qbuf(fd, i) < 0) {
            //LIB_LOG_ERR("VIDIOC_QBUF");
            return -1;
        }
    }

    if(xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        //LIB_LOG_ERR("VIDIOC_STREAMON");
        return -1;
    }

    return 0;
}

int v4l_qbuf(int fd, int idx) {
    struct v4l2_buffer buf = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP,
        .index = idx,
    };

    if(xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        LIB_LOG_ERR("VIDIOC_QBUF");
        return -1;
    }
    return 0;
}

int v4l_dqbuf(int fd, size_t *len) {
    struct v4l2_buffer buf = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP,
    };

    if(xioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        LIB_LOG_ERR("VIDIOC_DQBUF");
        return -1;
    }

    if(len) {
        if(xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            LIB_LOG_ERR("VIDIOC_QUERYBUF");
        }
        *len = buf.length;
    }

    return buf.index;
}

int v4l_poll(int fd, int timeout) {
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = POLLIN;

    return poll(&pfd, 1, timeout);
}
