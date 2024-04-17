#ifndef LIB_ERR_H_
#define LIB_ERR_H_

#include <stdbool.h>
/* see <linux/err.h> for reference */

#define IS_ERR_VALUE(x) ((unsigned long)(x) >= (unsigned long)-(0xfff))

static inline void* ERR_PTR(long err) \
{ return (void*)err; }

static inline long PTR_ERR(const void* p) \
{ return (long)p; }

static inline bool IS_ERR(const void* p) \
{ return IS_ERR_VALUE(p); }

static inline int IS_ERR_OR_NULL(const void* p) \
{ return ((!p) || IS_ERR_VALUE(p)); }

#endif