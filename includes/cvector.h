#ifndef CVECTOR_H_
#define CVECTOR_H_

#include <stddef.h>

/** void *ptrs refer to **obj but void **ptrs wouldn't work */
void freev(void *ptrs);
void *pushv(void *ptr, void *ptrs);
void *popv(void *ptrs);
void *popv_front(void *ptrs);
size_t countv(void *ptrs);

#endif