#ifndef CTYPES_H_
#define CTYPES_H_

#define _packed __attribute__((packed))
#define _unused __attribute__((unused))
#define _used __attribute__((used))
#define _align(x) __attribute__((aligned(x)))
#define _section(s) __attribute__((section(#s)))

#define _export(s) extern typeof(s) s;

#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a) ALIGN_MASK(x, (typeof(x))(a) - 1)

#define MIN(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	_x < _y ? _x : _y; })

#define MAX(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	_x > _y ? _x : _y; })

#define CLAMP(val, hi, lo) MIN((typeof(val)) MAX(val, lo), hi)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define GENMASK(h, l) (((~(0u)) - (1 << (l)) + 1) & (~(0u) >> (32 - 1 - (h))))
#define BIT(x) (1 << x)

#endif