
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <lib_log.h>
#include <lib_exttool.h>

static int ext_generic_execute(const char *cmd) {
    FILE *fp;

    fp = popen(cmd, "r");
    if(!fp)
        return -1;

    return WEXITSTATUS(pclose(fp));
}

static int ext_generic_read(const char *cmd, char *buf, size_t size) {
    FILE *fp;

    fp = popen(cmd, "r");
    if(!fp)
        return -1;

    if(!fgets(buf, size, fp))
        goto err;

    return WEXITSTATUS(pclose(fp));

err:
    pclose(fp);
    return -1;
}

int ext_backlight_power(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "echo %d > /sys/class/backlight/ncp5623/"
                "bl_power 2>/dev/null",
                !on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_backlight_brightness(int brightness) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "echo %d > /sys/class/backlight/ncp5623/"
                "brightness 2>/dev/null",
                brightness) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_mediactl(uint32_t w, uint32_t h, uint32_t fps) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "media-ctl -V \"\'ov5640 2-003c\':0 "
                "[fmt:YUYV8_1X16/%ux%u@1/%u]\" -d "
                "/dev/media0 2>/dev/null",
                w, h, fps) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_subdev_testpattern(int mode) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "test_pattern=%d 2>/dev/null",
                mode & 0x3) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_subdev_illu1(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "illuminator_1=%d 2>/dev/null",
                !!on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_subdev_illu2(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "illuminator_2=%d 2>/dev/null",
                !!on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_subdev_autofocus(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "focus_automatic_continuous=%d 2>/dev/null",
                !!on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_set_subdev_focus_value(int val) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "focus_relative=%d 2>/dev/null", val) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_get_subdev_focus_value(int *val) {
    char  buf[256];

    if(ext_generic_read("v4l2-ctl -d /dev/v4l-subdev0 -C "
                        "focus_relative 2>/dev/null",
                        buf, sizeof(buf)) < 0) {
        LIB_LOG_ERR("failed to read");
        return -1;
    }

    if(val)
        *val = strtol(buf, NULL, 0);

    return 0;
}

int ext_subdev_autoexposure(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "auto_exposure=%d 2>/dev/null",
                !on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_set_subdev_exposure_value(int val) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "exposure=%d 2>/dev/null", val) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_get_subdev_exposure_value(int *val) {
    char buf[256];

    if(ext_generic_read("v4l2-ctl -d /dev/v4l-subdev0 -C "
                        "exposure 2>/dev/null",
                        buf, sizeof(buf)) < 0) {
        LIB_LOG_ERR("failed to read");
        return -1;
    }

    if(val)
        *val = strtol(buf, NULL, 0);

    return 0;
}

int ext_subdev_autogain(int on) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "gain_automatic=%d 2>/dev/null",
                !!on) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_set_subdev_gain_value(int val) {
    char cmd[256];

    if(snprintf(cmd, sizeof(cmd),
                "v4l2-ctl -d /dev/v4l-subdev0 -c "
                "gain=%d 2>/dev/null", val) < 0)
        return -1;

    return ext_generic_execute(cmd);
}

int ext_get_subdev_gain_value(int *val) {
    char  buf[256];

    if(ext_generic_read("v4l2-ctl -d /dev/v4l-subdev0 -C "
                        "gain 2>/dev/null",
                        buf, sizeof(buf)) < 0) {
        LIB_LOG_ERR("failed to read");
        return -1;
    }

    if(val)
        *val = strtol(buf, NULL, 0);

    return 0;
}

int ext_mainboard_temperature(float *val) {
    int offset, raw, scale;
    char out[256];
    FILE *fp;

    fp = fopen("/sys/bus/iio/devices/iio:device0/"
               "in_temp0_ps_temp_offset", "r");
    if(!fp) {
        LIB_LOG_ERR("failed to open offset");
        return -1;
    }
    if(!fgets(out, sizeof(out), fp)) {
        LIB_LOG_ERR("failed to read offset");
        fclose(fp);
        return -1;
    }
    offset = strtol(out, NULL, 0);
    fclose(fp);


    fp = fopen("/sys/bus/iio/devices/iio:device0/"
               "in_temp0_ps_temp_raw", "r");
    if(!fp) {
        LIB_LOG_ERR("failed to open raw");
        return -1;
    }
    if(!fgets(out, sizeof(out), fp)) {
        LIB_LOG_ERR("failed to read raw");
        fclose(fp);
        return -1;
    }
    raw = strtol(out, NULL, 0);
    fclose(fp);

    fp = fopen("/sys/bus/iio/devices/iio:device0/in_temp0_ps_temp_scale", "r");
    if(!fp) {
        LIB_LOG_ERR("failed to open scale");
        return -1;
    }
    if(!fgets(out, sizeof(out), fp)) {
        LIB_LOG_ERR("failed to read scale");
        fclose(fp);
        return -1;
    }
    scale = strtol(out, NULL, 0);
    fclose(fp);

    if(val)
        *val = (float)((scale * (raw + offset)) / 1000.0f);

    return 0;
}
