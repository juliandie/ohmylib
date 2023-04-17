#ifndef OHMYLIB_H_
#define OHMYLIB_H_

#include <lib_crc.h>
#include <lib_err.h>
#include <lib_i2c.h>
#include <lib_lfsr.h>
#include <lib_log.h>
#include <lib_netif.h>
#include <lib_poll.h>
#include <lib_popen.h>
#include <lib_select.h>
#include <lib_sock.h>
#include <lib_sysreg.h>
#include <lib_time.h>
#include <lib_v4l2.h>
#include <list.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

 #define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)

#endif