#ifndef MODULE_H_
#define MODULE_H_

/** Why using this method to do initcalls?
 * As one wants to add some code that requires init- and exit-code
 * you don't need to add any init- and exit-calls to the existing
 * code-base. The module registers itself on an initcall-level
 * and is called during startup automatically.
 * In addition the overhead for this subsystem is negligible.
 */

/** How does modules work?
 * I took this idea from the Linux-Kernel. By assigning a custom
 * section to a function-call pointer gcc will create a dedicated
 * label. For example __attribute__((section("initcall_##id"))
 * which will result in section labels "__start_initcall_##id"
 * and "__stop_initcall_##id". So we're able to get all initcall
 * function-pointers for that defined ##id between those two labels.
 * Therefore an additional module must be only compiled-in, no further
 * changes in the code required to run the init code.
 */

#define module_init(fn)   user_initcall(fn)
#define module_exit(fn)   __exitcall(fn)

#define core_module(fn) core_initcall(fn); exitcall(fn)
#define user_module(fn) user_initcall(fn); exitcall(fn)
#define late_module(fn) late_initcall(fn); exitcall(fn)

#define core_initcall(fn) __define_initcall(fn, 1)
#define user_initcall(fn) __define_initcall(fn, 6)
#define late_initcall(fn) __define_initcall(fn, 7)

/** initcall */
#define __used       __attribute((__used__))
#define __section(s) __attribute((__section__(#s)))

#define ___define_initcall(fn, id, __sec) \
    static initcall_t __initcall_##fn##id __used\
        __attribute((__section__(#__sec "_init"))) = fn;
#define __define_initcall(fn, id) ___define_initcall(fn, id, initcall##id)
typedef int (*initcall_t)(void);
typedef initcall_t initcall_entry_t;
#define __init __used __section(_init_text)

/** exitcall */
#define __exit_call __used __section(exitcall_exit)
#define __exitcall(fn) \
    static exitcall_t __exitcall_##fn __exit_call = fn;

typedef void (*exitcall_t)(void);
typedef exitcall_t exitcall_entry_t;
#define __exit __used __section(.exit.text)

void do_initcalls(void);
void do_exitcalls(void);

#endif /* MODULE_H_ */
