#ifndef LIB_MODULE_H_
#define LIB_MODULE_H_
#if 0
typedef int (*mod_initcall_t)(void);
typedef void (*mod_exitcall_t)(void);

/** Collect init-function pointers in a separate section 'mod_initcall##id' */
#define mod_define_initcall(fn, id) \
    static __attribute((__used__)) mod_initcall_t \
    __attribute((__section__(mod_initcall##id))) \
    mod_init_##fn##id = fn;

#define mod_core_initcall(fn) mod_define_initcall(fn, 0)

#define mod_init_code __attribute((__used__)) \
    __attribute((__section__("mod_initcode")))



#define mod_exit_code __attribute((__used__)) \
    __attribute((__section__("mod_exitcode")))

#define mod_define_exitcall(fn, id) \
    static __attribute((__used__)) mod_exitcall_t \
    __attribute((__section__(mod_exitcall##id))) \
    mod_exit_##fn = fn;

#define mod_exitcall(fn) mod_define_exitcall(fn, 0)

#define module_register(fn) \
    mod_core_initcall(fn##_init); \
    mod_exitcall(fn##_exit); 

void module_initcall(void);
void module_exitcall(void);
#endif
#endif