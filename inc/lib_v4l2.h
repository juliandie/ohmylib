#ifndef OHMYLIB_V4L2_H_
#define OHMYLIB_V4L2_H_

#include <stdint.h> // size_t
#include <linux/videodev2.h>
#include <lib_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif // defined (__cplusplus)

int v4l_close(int fd);
int v4l_open(const char *videodev);
int v4l_set_format(int fd, struct v4l2_pix_format *format);
int v4l_get_format(int fd, struct v4l2_pix_format *format);
void v4l_munmap_buffer(int fd, void ***p, size_t n_buf);
int v4l_mmap_buffer(int fd, void ***p, size_t n_buf);
int v4l_stop(int fd);
int v4l_start(int fd, size_t n_buf);
int v4l_qbuf(int fd, int idx);
int v4l_dqbuf(int fd, size_t *len);
int v4l_poll(int fd, int timeout);

#ifdef __cplusplus
}
#endif // defined (__cplusplus)

#endif