
#include <lib_buffer.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


/** Create a buffer handle and initialize it with a buffer of size */
lib_buffer *lib_buffer_create(size_t size) {
    lib_buffer *buf;

    buf = malloc(sizeof(lib_buffer));
    if(!buf)
        return NULL;

    if(lib_buffer_alloc(buf, size) < 0) {
        goto free_buf;
    }

    return buf;

free_buf:
    free(buf);
    return NULL;
}

/** Create a buffer of size in an empty buffer handle */
int lib_buffer_alloc(lib_buffer *buf, size_t size) {
    if(!buf)
        return -1;

    if(!size)
        return 0;

    memset(buf, 0, sizeof(lib_buffer));
#if 0 // TODO double check if needed for Gig-E-Vision
    if(size & 0x1f) { // align 32B
        size &= 0x1f;
        size += 0x20;
    }
#endif
    buf->data = malloc(size);
    if(!buf->data)
        return -1;

    memset(buf->data, 0, size);
    buf->size = size;
    return 0;
}

/** Set a buffer and its size in an empty buffer handle */
int lib_buffer_set(lib_buffer *buf, char *data, size_t size) {
    if(!buf || !data || !size)
        return -1;

    memset(buf, 0, sizeof(lib_buffer));
    buf->data = data;
    buf->size = size;
    return 0;
}

/** Cleanup the buffer of an allocated buffer handle */
void lib_buffer_free(lib_buffer *buf) {
    if(!buf)
        return;

    if(buf->data) {
        free(buf->data);
    }
    memset(buf, 0, sizeof(lib_buffer));
}

/** Cleanup the contents of the buffer */
void lib_buffer_clear(lib_buffer *buf) {
    if(!buf || !buf->data || !buf->size)
        return;

    memset(buf->data, 0, buf->size);
    buf->offs = 0;
    buf->len = 0;
}

/** Duplicate a buffer handle into a empty buffer handle */
lib_buffer *lib_buffer_dup(lib_buffer *src) {
    lib_buffer *buf;

    buf = lib_buffer_create(src->size);
    if(!buf)
        return NULL;

    if(lib_buffer_cpy(buf, src) < 0) {
        lib_buffer_destroy(buf);
        return NULL;
    }

    buf->offs = src->offs;

    return buf;
}

/** Copy the contents from a buffer handle to another buffer handle */
int lib_buffer_cpy(lib_buffer *dst, lib_buffer *src) {
    if(!dst || !src || dst->size < src->len)
        return -EINVAL;

    if(!src->size || !src->len)
        return 0;

    memcpy(dst->data, src->data, src->size);
    dst->len = src->len;
    return 0;
}

/** Cleanup a buffer handle */
void lib_buffer_destroy(lib_buffer *buf) {
    if(!buf)
        return;

    lib_buffer_free(buf);

    free(buf);
}

/** Resize a buffer handle to size (size = 0 results in lib_buffer_free) */
int lib_buffer_resize(lib_buffer *buf, size_t size) {
    char *p;

    if(!buf)
        return -1;

    if(size == 0) {
        lib_buffer_free(buf);
        return 0;
    }

    p = realloc(buf->data, size);
    if(!p)
        return -1;

    buf->data = p;
    buf->size = size;
    return 0;
}

uint16_t lib_buffer_gets(lib_buffer *buf) {
    uint16_t ret;

    if(!buf)
        return 0;

    if(buf->offs + (int)sizeof(uint16_t) > buf->size)
        return 0;

    memcpy(&ret, buf->data + buf->offs, 2);
    buf->offs += sizeof(uint16_t);

    return ret;
}

uint32_t lib_buffer_geti(lib_buffer *buf) {
    uint32_t ret;

    if(!buf)
        return 0;

    if(buf->offs + (int)sizeof(uint32_t) > buf->size)
        return 0;

    memcpy(&ret, buf->data + buf->offs, 4);
    buf->offs += sizeof(uint32_t);

    return ret;
}

/** Copy data of size into the buffer handle */
int lib_buffer_puta(lib_buffer *buf, const void *a, size_t size) {
    if(!buf || !a)
        return -EINVAL;

    if(buf->size - buf->len < size)
        return -E2BIG;

    if(size == 0)
        goto done;

    memcpy(buf->data + buf->offs, a, size);
    buf->len += size;
    buf->offs += size;
done:
    return size;
}

/** Copy a 8b value into the buffer handle */
int lib_buffer_putc(lib_buffer *buf, uint8_t v) {
    return lib_buffer_puta(buf, &v, sizeof(uint8_t));
}

/** Copy a 16b value into the buffer handle */
int lib_buffer_puts(lib_buffer *buf, uint16_t v) {
    return lib_buffer_puta(buf, &v, sizeof(uint16_t));
}

/** Copy a 32b value into the buffer handle */
int lib_buffer_puti(lib_buffer *buf, uint32_t v) {
    return lib_buffer_puta(buf, &v, sizeof(uint32_t));
}

/** Copy a 64b value into the buffer handle */
int lib_buffer_putl(lib_buffer *buf, uint64_t v) {
    return lib_buffer_puta(buf, &v, sizeof(uint64_t));
}

/** Return the current offset of a buffer handle */
int lib_buffer_tell(lib_buffer *buf) {
    return buf->offs;
}

/** Seek the offset of a buffer handle */
int lib_buffer_seek(lib_buffer *buf, off_t offset, int whence) {
    if(!buf) {
        errno = EINVAL;
        return -1;
    }

    switch(whence) {
    case SEEK_SET:
        if(offset < 0 || offset >(off_t)buf->size) {
            errno = EINVAL;
            return -1;
        }
        buf->offs = (size_t)offset;
        break;

    case SEEK_END:
        if(offset > 0 || ((off_t)buf->size - offset) < 0) {
            errno = EINVAL;
            return -1;
        }
        buf->offs = buf->size + (size_t)offset;
        break;

    case SEEK_CUR:
        if(offset > 0 && ((off_t)buf->offs + offset) < (off_t)buf->size)
            buf->offs += offset;
        else if(offset < 0 && buf->offs + offset > 0)
            buf->offs += offset;
        else {
            errno = EINVAL;
            return -1;
        }
        break;

    default:
        errno = EINVAL;
        return -1;
    }
    return 0;
}

void lib_buffer_append(lib_buffer *buf, size_t count) {
    if(!buf || !count || buf->size < (buf->len + count))
        return;

    buf->len += count;
    buf->offs += count;
}

void lib_buffer_info(lib_buffer *buf) {
    printf("%p+%zuB; %zuB/%zuB\n", buf->data, buf->offs, buf->len, buf->size);
}
