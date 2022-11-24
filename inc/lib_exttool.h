
#ifndef OHMYLIB_EXTTOOL_H_
#define OHMYLIB_EXTTOOL_H_

#include <stdint.h>
#include <unistd.h>

int ext_backlight_power(int on);
int ext_backlight_brightness(int brightness);

int ext_mediactl(uint32_t w, uint32_t h, uint32_t fps);
int ext_subdev_testpattern(int mode);
int ext_subdev_illu1(int on);
int ext_subdev_illu2(int on);

int ext_subdev_autofocus(int on);
int ext_set_subdev_focus_value(int val);
int ext_get_subdev_focus_value(int *val);

int ext_subdev_autoexposure(int on);
int ext_set_subdev_exposure_value(int val);
int ext_get_subdev_exposure_value(int *val);

int ext_subdev_autogain(int on);
int ext_set_subdev_gain_value(int val);
int ext_get_subdev_gain_value(int *val);

int ext_mainboard_temperature(float *val);

#endif
