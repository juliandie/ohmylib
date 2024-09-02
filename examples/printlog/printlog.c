#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <errno.h>
#include <string.h>

#include <printlog.h>

int main() {
    const char buf[] = {'H', 'e', 'l', 'l', 'o', ' ',
                        'W', 'o', 'r', 'l', 'd', 0x0,
                        0x1, 0x2, 0x3, 0x4, 0x5, 0x6};

    fdump(stderr, buf, sizeof(buf), "fdump prefix");
    fhexdump(stderr, buf, sizeof(buf), "fhexdump prefix");

    PRINTLOG_EMERG("emergency, system is unusable");
    PRINTLOG_ALERT("alert, action must be taken immediately");
    PRINTLOG_CRIT("we have some critical conditions");
    PRINTLOG_ERR("some error condition");
    PRINTLOG_WARNING("some warning conditions");
    PRINTLOG_NOTICE("normal, but significant, condition");
    PRINTLOG_INFO("informational message");
    PRINTLOG_DEBUG("debug message");

    return 0;
}