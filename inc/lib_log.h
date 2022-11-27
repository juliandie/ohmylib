
#ifndef LIB_LOG_H_
#define LIB_LOG_H_

#include <stdio.h>  // printf
#include <string.h> // strerror
#include <errno.h>  // errno
#include <time.h>   // timespec, clock_gettime
#ifdef LIB_LOG_SYSLOG
#include <syslog.h> // see man syslog
#else // usually defined in syslog
#define LOG_EMERG   (0u) // system is unusable
#define LOG_ALERT   (1u) // action must be taken immediately
#define LOG_CRIT    (2u) // critical conditions
#define LOG_ERR     (3u) // error conditions
#define LOG_WARNING (4u) // warning conditions
#define LOG_NOTICE  (5u) // normal, but significant, condition
#define LOG_INFO    (6u) // informational message
#define LOG_DEBUG   (7u) // debug-level message
#endif

// default to INFO loglevel
#ifndef LOG_LEVEL
#define LOG_LEVEL (LOG_DEBUG)
#endif

void lib_dump(const void* p, size_t size, const char* fmt, ...);
void lib_hexdump(const void *p, size_t size, const char *fmt, ...);

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_EMERG)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_EMERG(fmt, ...) \
	{ syslog(LOG_EMERG, "[EMERG] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_EMERG(fmt, ...) \
	{ fprintf(stderr, "[EMERG] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_EMERG(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_ALERT)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_ALERT(fmt, ...) \
	{ syslog(LOG_ALERT, "[ALERT] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_ALERT(fmt, ...) \
	{ fprintf(stderr, "[ALERT] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_ALERT(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_CRIT)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_CRIT(fmt, ...) \
	{ syslog(LOG_CRIT, "[CRIT] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_CRIT(fmt, ...) \
	{ fprintf(stderr, "[CRIT] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_CRIT(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_ERR)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_ERR(fmt, ...) \
	{ syslog(LOG_ERR, "[ERR] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_ERR(fmt, ...) \
	{ fprintf(stderr, "[ERR] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_ERR(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_WARNING)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_WARNING(fmt, ...) \
	{ syslog(LOG_WARNING, "[WARNING] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_WARNING(fmt, ...) \
	{ fprintf(stderr, "[WARNING] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_WARNING(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_NOTICE)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_NOTICE(fmt, ...) \
	{ syslog(LOG_NOTICE, "[NOTICE] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_NOTICE(fmt, ...) \
	{ fprintf(stderr, "[NOTICE] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_NOTICE(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_INFO)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_INFO(fmt, ...) \
	{ syslog(LOG_INFO, "[INFO] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_INFO(fmt, ...) \
	{ fprintf(stderr, "[INFO] %s "fmt"\n", __func__, ##__VA_ARGS__);}
#endif
#else
#define LIB_LOG_INFO(fmt, ...) {}
#endif

#if defined(LOG_LEVEL) && (LOG_LEVEL >= LOG_DEBUG)
#ifdef LIB_LOG_SYSLOG
#define LIB_LOG_DEBUG(fmt, ...) \
	{ struct timespec tp; \
	  clock_gettime(CLOCK_REALTIME, &tp); \
	  syslog(LOG_DEBUG, "[DEBUG] %lu %s(%s):%d -- "fmt"\n", \
	  	((tp.tv_sec * 1000000000ul) + tp.tv_nsec) / 1000, \
	  	__func__, __FILE__, __LINE__, ##__VA_ARGS__); }
#else // !def LIB_LOG_SYSLOG
#define LIB_LOG_DEBUG(fmt, ...) \
	{ struct timespec tp; \
	  clock_gettime(CLOCK_REALTIME, &tp); \
	  fprintf(stderr, "[DEBUG] %lu %s(%s):%d -- "fmt"\n", \
	  	((tp.tv_sec * 1000000000ul) + tp.tv_nsec) / 1000, \
	  	__func__, __FILE__, __LINE__, ##__VA_ARGS__); }
#endif
#else
#define LIB_LOG_DEBUG(fmt, ...) {}
#endif

#endif