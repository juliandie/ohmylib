#include <stdarg.h> // va_start, va_end

#include <printlog.h>

void dump(const void *p, size_t size, const char *fmt, ...) {
    char *buf;
    size_t len;

    if(fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }

    printf("   ");
    for(int i = 0; i < 16; i++) {
        printf("%02x ", i);
    }
    printf("\n");

    buf = (char *)p;
    len = size;

    while(len) {
        if(((size - len) % 0x10) == 0)
            printf("%s%02x ",
                   ((size - len) == 0) ? "" : "\n",
                   (int)((size - len) & 0xff));

        for(int i = 0; i < 0x10; i++) {
            char val = *buf;

            if(len > 0) {
                printf("%02x ", val & 0xff);
                len--;
                buf++;
            }
        }
    }
    printf("\n");
    fflush(stdout);
}

void fdump(FILE *f, const void *p, size_t size, const char *fmt, ...) {
    char *buf;
    size_t len;

    if(fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(f, fmt, ap);
        va_end(ap);
        fprintf(f, "\n");
    }

    fprintf(f, "   ");
    for(int i = 0; i < 16; i++) {
        fprintf(f, "%02x ", i);
    }
    fprintf(f, "\n");

    buf = (char *)p;
    len = size;

    while(len) {
        if(((size - len) % 0x10) == 0)
            fprintf(f, "%s%02x ",
                    ((size - len) == 0) ? "" : "\n",
                    (int)((size - len) & 0xff));

        for(int i = 0; i < 0x10; i++) {
            char val = *buf;

            if(len > 0) {
                fprintf(f, "%02x ", val & 0xff);
                len--;
                buf++;
            }
        }
    }
    fprintf(f, "\n");
    fflush(f);
}

void hexdump(const void *p, size_t size, const char *fmt, ...) {
    char *hbuf, *cbuf;
    size_t hlen, tlen;

    if(fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }

    printf("     ");
    for(int i = 0; i < 16; i++) {
        printf("%02x ", i);
    }
    printf("\n");

    hlen = tlen = size;
    hbuf = cbuf = (char *)p;

    while(tlen) {
        if(((size - tlen) % 0x10) == 0)
            printf("%s%04x ",
                   ((size - tlen) == 0) ? "" : "\n",
                   (int)((size - tlen)));

        for(int i = 0; i < 0x10; i++) {
            char val = *hbuf;

            if(hlen > 0) {
                printf("%02x ", val & 0xff);
                hlen--;
                hbuf++;
            }
            else
                printf("   ");
        }
        printf("  ");
        for(int i = 0; i < 0x10; i++) {
            char val = *cbuf;
            if(((size - tlen) % 0x8) == 0)
                printf(" ");

            if(tlen > 0) {
                if(val > 0x20 && val < 0x7e)
                    printf("%c", val);
                else
                    printf(".");
                tlen--;
                cbuf++;
            }
        }
    }
    printf("\n");
    fflush(stdout);
}

void fhexdump(FILE *f, const void *p, size_t size, const char *fmt, ...) {
    char *hbuf, *cbuf;
    size_t hlen, tlen;

    if(fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(f, fmt, ap);
        va_end(ap);
        fprintf(f, "\n");
    }
    fflush(f);

    fprintf(f, "     ");
    for(int i = 0; i < 16; i++) {
        fprintf(f, "%02x ", i);
    }
    fprintf(f, "\n");
    fflush(f);

    hlen = tlen = size;
    hbuf = cbuf = (char *)p;

    while(tlen) {
        if(((size - tlen) % 0x10) == 0)
            fprintf(f, "%s%04x ",
                    ((size - tlen) == 0) ? "" : "\n",
                    (int)((size - tlen)));

        for(int i = 0; i < 0x10; i++) {
            char val = *hbuf;

            if(hlen > 0) {
                fprintf(f, "%02x ", val & 0xff);
                hlen--;
                hbuf++;
            }
            else
                fprintf(f, "   ");
        }
        fprintf(f, "  ");
        for(int i = 0; i < 0x10; i++) {
            char val = *cbuf;
            if(((size - tlen) % 0x8) == 0)
                fprintf(f, " ");

            if(tlen > 0) {
                if(val > 0x20 && val < 0x7e)
                    fprintf(f, "%c", val);
                else
                    fprintf(f, ".");
                tlen--;
                cbuf++;
            }
        }
    }
    fprintf(f, "\n");
    fflush(f);
}
