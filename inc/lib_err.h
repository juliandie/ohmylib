#ifndef LIB_ERR_H_
#define LIB_ERR_H_

// see <linux/err.h>

#define MAX_ERRNO	(4095)

#define IS_ERR_VALUE(x) ((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void * ERR_PTR(long error) {
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr) {
	return (long) ptr;
}

static inline bool IS_ERR(const void *ptr) {
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline bool IS_ERR_OR_NULL(const void *ptr) {
	return (ptr == NULL) || IS_ERR_VALUE((unsigned long)ptr);
}

static inline int PTR_ERR_OR_ZERO(const void *ptr) {
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}

#endif /* LIB_ERR_H_ */