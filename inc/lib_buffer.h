
#ifndef OHMYLIB_BUFFER_H_
#define OHMYLIB_BUFFER_H_

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Buffer shouldn't exceed 0x80000000 (2147483648B)
 */

typedef struct lib_buffer_s {
	char* data;
	size_t size; // size of buffer
	size_t len; // data written to data in Byte
	size_t offs; // data read from data in Byte
} lib_buffer;

lib_buffer* lib_buffer_create(size_t size);
int lib_buffer_alloc(lib_buffer* buf, size_t size);
int lib_buffer_set(lib_buffer* buf, char* data, size_t size);
void lib_buffer_free(lib_buffer* buf);
lib_buffer* lib_buffer_dup(lib_buffer* src);
int lib_buffer_cpy(lib_buffer* dst, lib_buffer *src);
void lib_buffer_destroy(lib_buffer* buf);

int lib_buffer_resize(lib_buffer* buf, size_t size);
void lib_buffer_clear(lib_buffer* buf);

uint16_t lib_buffer_gets(lib_buffer* buf);
uint32_t lib_buffer_geti(lib_buffer* buf);

int lib_buffer_puta(lib_buffer* buf, const void* a, size_t size);
int lib_buffer_putc(lib_buffer* buf, uint8_t v);
int lib_buffer_puts(lib_buffer* buf, uint16_t v);
int lib_buffer_puti(lib_buffer* buf, uint32_t v);
int lib_buffer_putl(lib_buffer* buf, uint64_t v);

int lib_buffer_tell(lib_buffer* buf);
int lib_buffer_seek(lib_buffer* buf, off_t offset, int whence);
void lib_buffer_append(lib_buffer* buf, size_t count);

void lib_buffer_info(lib_buffer* buf);

#endif