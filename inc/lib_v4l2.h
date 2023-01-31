#ifndef OHMYLIB_V4L2_H_
#define OHMYLIB_V4L2_H_

#include <stdint.h>
#include <linux/videodev2.h>

#ifdef __cplusplus
extern "C" {
#endif // defined (__cplusplus)

int v4l_close(int fd);
int v4l_open(const char *videodev);
int v4l_set_format(int fd, struct v4l2_pix_format *format);
int v4l_get_format(int fd, struct v4l2_pix_format *format);
void v4l_munmap_buffer(int fd, void ***p, size_t n_buf);
int v4l_mmap_buffer(int fd, void ***p, size_t n_buf);
//void v4l_free_buffer(int fd, void ***p, size_t n_buf);
//int v4l_alloc_buffer(int fd, void ***p, size_t n_buf);
int v4l_stop(int fd);
int v4l_start(int fd, size_t n_buf);
int v4l_qbuf(int fd, int idx);
int v4l_dqbuf(int fd, size_t *len);
int v4l_poll(int fd, int timeout);

#ifdef __cplusplus
}
#endif // defined (__cplusplus)

/** Sample
 * capture 10 images from /dev/video0
 * use mmap for buffers
 * set resolution is 1920x1080
 * set colorspace RGB32
 * set 3 buffer

int main() {
    int fd;
    void *buf[3];
    int buf_count, buf_idx;
    struct v4l2_pix_format format = {
        .width = 1920,
        .height = 1080,
        .pixelformat = V4L2_PIX_FMT_RGB32,
        .field = V4L2_FIELD_ANY,
    };

    fd = v4l_open("/dev/video0");
    if(fd < 0)
        return -1;

    if(v4l_set_format(fd, &format) < 0)
        goto err_close;
    
    buf_count = v4l_mmap_buffer(fd, &buf, 3);
    if(buf_count < 0)
        goto err_close;
    
    if(v4l_start(fd, buf_count) < 0)
        goto err_munmap;

    for(int i = 0; i < 10; i++) {
        if(v4l_poll(fd, 1000) <= 0)
            goto err_stop;

        buf_idx = v4l_dqbuf(fd);
        if(buf_idx < 0)
            goto err_stop;

        // do something with buf[buf_idx];

        if(v4l_qbuf(fd, buf_idx) < 0)
            goto err_stop;
    }
    
    v4l_stop(fd);
    v4l_munmap_buffer(fd, &buf, buf_count);
    v4l_close(fd);

    return 0;

err_stop:
    v4l_stop(fd);
err_munmap:
    v4l_munmap_buffer(fd, &buf, buf_count);
err_close:
    v4l_close(fd);
    return -1;
}
 */
#endif