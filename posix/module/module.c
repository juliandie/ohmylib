#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if 0
#include <execinfo.h>
#endif

#include <module.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

struct initcall_info {
    initcall_entry_t *start;
    initcall_t *stop;
};

extern initcall_t __start_initcall1_init[], __stop_initcall1_init[];
#if 0
extern initcall_t __start_initcall2_init[], __stop_initcall2_init[];
extern initcall_t __start_initcall3_init[], __stop_initcall3_init[];
extern initcall_t __start_initcall4_init[], __stop_initcall4_init[];
extern initcall_t __start_initcall5_init[], __stop_initcall5_init[];
#endif
extern initcall_t __start_initcall6_init[], __stop_initcall6_init[];
extern initcall_t __start_initcall7_init[], __stop_initcall7_init[];

static struct initcall_info initcall_levels[] = {
    {__start_initcall1_init, __stop_initcall1_init},
#if 0
    {__start_initcall2_init, __stop_initcall2_init},
    {__start_initcall3_init, __stop_initcall3_init},
    {__start_initcall4_init, __stop_initcall4_init},
    {__start_initcall5_init, __stop_initcall5_init},
#endif
    {__start_initcall6_init, __stop_initcall6_init},
    {__start_initcall7_init, __stop_initcall7_init},
};

int do_one_initcall(initcall_t fn) {
#if 0
    char **function_name;
    void *function_pointer = fn;
#endif
    int ret;

#if 0
    function_name = backtrace_symbols(&function_pointer, 1);
    //do_trace_initcall_start(fn);
    GV_LOG_INFO("initcall %s", function_name[0]);
#endif
    ret = fn();
    //do_trace_initcall_finish(fn, ret);

#if 0
    if(ret) {
        fprintf(stderr, "initcall %s returned %d\n", function_name[0], ret);
    }
#endif
    return ret;
}

static inline initcall_t __init initcall_from_entry(initcall_entry_t *e) {
    return *e;
}

static void __init do_initcall_level(int level) {
    initcall_entry_t *fn;

    for(fn = initcall_levels[level].start;
        fn < initcall_levels[level].stop; fn++)
        do_one_initcall(initcall_from_entry(fn));
}

void __init do_initcalls(void) {
    int level;

    for(level = 0; level < (int)ARRAY_SIZE(initcall_levels); level++) {
        do_initcall_level(level);
    }
}

static int dummy_init() {
    return 0;
}

core_initcall(dummy_init);
user_initcall(dummy_init);
late_initcall(dummy_init);

struct exitcall_info_s {
    exitcall_entry_t *start;
    exitcall_t *stop;
};

extern exitcall_t __start_exitcall_exit[], __stop_exitcall_exit[];

static struct exitcall_info_s exitcalls[] = {
    {__start_exitcall_exit, __stop_exitcall_exit},
};

static void do_one_exitcall(exitcall_t fn) {
    //do_trace_exitcall_start(fn);
    fn();
    //do_trace_exitcall_finish(fn, ret);
}

static inline exitcall_t exitcall_from_entry(exitcall_entry_t *e) {
    return *e;
}

void  do_exitcalls(void) {
    exitcall_entry_t *fn;

    for(fn = exitcalls->start; fn < exitcalls->stop; fn++)
        do_one_exitcall(exitcall_from_entry(fn));
}

static void dummy_exit() {
    return;
}

module_exit(dummy_exit);

#if 0
void do_initcalls(void) {
    for(initcall_t *iter = &__start_my_initcalls;
        iter < &__stop_my_initcalls; iter++) {
        int ret = (*iter)();
        if(ret) {
            printf("[%s] (%d): %s\n", __FILE__, __LINE__, __func__);
        }
    }
}

void do_exitcalls(void) {
    for(exitcall_t *iter = &__start_my_exitcalls;
        iter < &__stop_my_exitcalls; iter++) {
        (*iter)();
    }
}

struct list_head *get_modules() {
    return &rms;
}
#endif