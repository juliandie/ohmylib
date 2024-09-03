#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <module.h>

//static int func(char *arg, size_t len) {
//    (void)arg;
//    (void)len;
//    printf("\t[%s] (%d): %s\n", __FILE__, __LINE__, __func__);
//    return 0;
//}
//
//static struct module_s mod = {
//    .name = __FILE__,
//    .callback = func,
//};

static int __init mod1_init(void) {
    //ret = module_register(&mod);
    printf("%s/%s:%d\n", __FILE__, __func__, __LINE__);
    return 0;
}

static void __exit mod1_exit(void) {
    //module_remove(&mod);
    printf("%s/%s:%d\n", __FILE__, __func__, __LINE__);
}

module_init(mod1_init);
module_exit(mod1_exit);

