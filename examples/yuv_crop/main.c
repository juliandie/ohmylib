#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include <lib_log.h>

static struct v4l2_rect crop = {
    .left = 800,
    .top = 380,
    .width = 320,
    .height = 240,
};

struct yuyv {
    union {
        uint32_t raw;
        struct {
            uint8_t y0;
            uint8_t u;
            uint8_t y1;
            uint8_t v;
        };
    };
};

struct rgb32 {
    union {
        uint32_t raw;
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };
    };
};

static inline int color_clamp(int x) {
    if(x > 255) return 255;
    if(x < 0)   return 0;
    return x;
}

#define YUV2R(y, u, v) color_clamp((298 * (y-16) + 409 * (v-128) + 128) >> 8)
#define YUV2G(y, u, v) color_clamp((298 * (y-16) - 100 * (u-128) - 208 * (v-128) + 128) >> 8)
#define YUV2B(y, u, v) color_clamp((298 * (y-16) + 516 * (u-128) + 128) >> 8)

static void yuyv_to_rgb32(const void *src, void *dst, uint32_t w, uint32_t h) {
    const struct yuyv *yuyv = (const struct yuyv *)src;
    struct rgb32 *rgb32 = (struct rgb32 *)dst;
    for(uint32_t y = 0; y < h; y++) {
        for(uint32_t x = 0; x < w; x += 2) {
            rgb32->r = YUV2R(yuyv->y0, yuyv->u, yuyv->v) & 0xff;
            rgb32->g = YUV2G(yuyv->y0, yuyv->u, yuyv->v) & 0xff;
            rgb32->b = YUV2B(yuyv->y0, yuyv->u, yuyv->v) & 0xff;
            rgb32++;
            rgb32->r = YUV2R(yuyv->y1, yuyv->u, yuyv->v) & 0xff;
            rgb32->g = YUV2G(yuyv->y1, yuyv->u, yuyv->v) & 0xff;
            rgb32->b = YUV2B(yuyv->y1, yuyv->u, yuyv->v) & 0xff;
            rgb32++;
            yuyv++;
        }
    }
}

static void rgb32_rotate(void *src, void *dst, int w, int h) {
    for(int x = 0; x < w; x++) {
        for(int y = h - 1; y >= 0; y--) {
            memcpy(dst, src + (y * w * 4 + x * 4), 4);
            dst += 4;
        }
    }
}

static void yuyv_crop(void *src, void *dst,
                      const struct v4l2_rect *area, uint32_t w) {
    for(uint32_t y = area->top; y < area->top + area->height; y++) {
        memcpy(dst, src + (y * w * 2 + area->left * 2), area->width * 2);
        dst += area->width * 2;
    }
}

static void rgb_store(void *buf, size_t len) {
    char fn[256];
    size_t size;
    int fd;
    int ret;

    if(snprintf(fn, sizeof(fn), "frame.rgb") < 0) {
        LIB_LOG_ERR("Couldn't set filename");
        return;
    }

    fd = open(fn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd < 0) {
        LIB_LOG_ERR("Couldn't open file %s", fn);
        return;
    }

    size = 0;
    while(size < len) {
        ret = write(fd, buf + size, len);
        if(ret < 0) {
            LIB_LOG_ERR("Couldn't write file");
            close(fd);
            return;
        }
        size += (size_t)ret;
    }
    close(fd);
}

static void yuyv_load(void *buf, size_t len) {
    char fn[256];
    size_t size;
    int fd;
    int ret;

    if(snprintf(fn, sizeof(fn), "frame.yuv") < 0) {
        LIB_LOG_ERR("Couldn't set filename");
        return;
    }

    fd = open(fn, O_RDWR);
    if(fd < 0) {
        LIB_LOG_ERR("Couldn't open file %s", fn);
        return;
    }

    size = 0;
    while(size < len) {
        ret = read(fd, buf + size, len);
        if(ret < 0) {
            LIB_LOG_ERR("Couldn't write file");
            close(fd);
            return;
        }
        size += (size_t)ret;
    }
    close(fd);
}

int main() {
    char buf[1920 * 1080 * 2];
    char cro[240 * 320 * 4];
    char rgb[240 * 320 * 4];
    //char rot[1920 * 1080 * 4];
    //char rgb[1920 * 1080 * 4];

    yuyv_load(buf, sizeof(buf));

    yuyv_crop(buf, cro, &crop, 1920);
    yuyv_to_rgb32(cro, rgb, crop.width, crop.height);
    rgb32_rotate(rgb, cro, crop.width, crop.height);

    //yuyv_to_rgb32(buf, rgb, 1920, 1080);
    //rgb32_rotate(rgb, rot, 1920, 1080);

    rgb_store(rgb, sizeof(cro));
    printf("stored\n");

    return 0;
}