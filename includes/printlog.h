
#ifndef PRINTLOG_H_
#define PRINTLOG_H_

#include <stdio.h>  // printf
#include <string.h> // strerror
#include <errno.h>  // errno
#include <time.h>   // timespec, clock_gettime
#ifdef PRINTLOG_SYSLOG
#include <syslog.h> // see man syslog
#else // usually defined in syslog
#define LOG_EMERG   (0u) // system is unusable
#define LOG_ALERT   (1u) // action must be taken immediately
#define LOG_CRIT    (2u) // critical conditions
#define LOG_ERR     (3u) // error conditions
#define LOG_WARNING (4u) // warning conditions
#define LOG_NOTICE  (5u) // normal, but significant, condition
#define LOG_INFO    (6u) // informational message
#define LOG_DEBUG   (7u) // debug message
#endif

// default to LOG_INFO
#ifndef LOG_LEVEL
#define LOG_LEVEL (LOG_INFO)
#endif

void dump(const void* p, size_t size, const char* fmt, ...);
void fdump(FILE *f, const void *p, size_t size, const char *fmt, ...);
void hexdump(const void *p, size_t size, const char *fmt, ...);
void fhexdump(FILE *f, const void *p, size_t size, const char *fmt, ...);

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_EMERG)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_EMERG(fmt, ...) \
	{ syslog(LOG_EMERG, "[EMERG] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_EMERG(fmt, ...) \
	{ fprintf(stderr, "\x1B[31m[EMERG] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_EMERG(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_ALERT)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_ALERT(fmt, ...) \
	{ syslog(LOG_ALERT, "[ALERT] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_ALERT(fmt, ...) \
	{ fprintf(stderr, "\x1B[31m[ALERT] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_ALERT(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_CRIT)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_CRIT(fmt, ...) \
	{ syslog(LOG_CRIT, "[CRIT] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_CRIT(fmt, ...) \
	{ fprintf(stderr, "\x1B[31m[CRIT] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_CRIT(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_ERR)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_ERR(fmt, ...) \
	{ syslog(LOG_ERR, "[ERROR] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_ERR(fmt, ...) \
	{ fprintf(stderr, "\x1B[31m[ERROR] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_ERR(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_WARNING)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_WARNING(fmt, ...) \
	{ syslog(LOG_WARNING, "[WARNING] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_WARNING(fmt, ...) \
	{ fprintf(stderr, "\x1B[33m[WARNING] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_WARNING(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_NOTICE)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_NOTICE(fmt, ...) \
	{ syslog(LOG_NOTICE, "[NOTICE] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_NOTICE(fmt, ...) \
	{ fprintf(stderr, "\x1B[37m[NOTICE] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_NOTICE(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_INFO)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_INFO(fmt, ...) \
	{ syslog(LOG_INFO, "[INFO] "fmt"\n", ##__VA_ARGS__);}
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_INFO(fmt, ...) \
	{ fprintf(stderr, "\x1B[32m[INFO] "fmt"\x1B[0m\n", ##__VA_ARGS__);}
#endif
#else
#define PRINTLOG_INFO(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_DEBUG)
#ifdef PRINTLOG_SYSLOG
#define PRINTLOG_DEBUG(fmt, ...) \
	{ struct timespec tp; \
	  clock_gettime(CLOCK_REALTIME, &tp); \
	  syslog(LOG_DEBUG, "[DEBUG] %lu %s(%s):%d -- "fmt"\n", \
	  	((tp.tv_sec * 1000000000ul) + tp.tv_nsec) / 1000, \
	  	__func__, __FILE__, __LINE__, ##__VA_ARGS__); }
#else // !def PRINTLOG_SYSLOG
#define PRINTLOG_DEBUG(fmt, ...) \
	{ fprintf(stderr, "\x1B[36m[DEBUG] %s:%d[%s] -- "fmt"\x1B[0m\n", \
	  	__FILE__, __LINE__, __func__, ##__VA_ARGS__); }
#endif
#else
#define PRINTLOG_DEBUG(fmt, ...) {}
#endif

#endif