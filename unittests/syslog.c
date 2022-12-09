
#include <string.h>

#define LIB_DBG (8)
#define LIB_LOG_SYSLOG
#include <lib_log.h>

const char dummy[] = {
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};

int main() {
    openlog(NULL, (LOG_PID | LOG_CONS | LOG_PERROR), LOG_LPR);
    LIB_LOG_EMERG("An emergency message!");
    LIB_LOG_ALERT("Be alarmed!");
    LIB_LOG_CRIT("Some critical issue appeared.");
    LIB_LOG_ERR("An error happened.");
    LIB_LOG_WARNING("Watchout some warnings ahead.");
    LIB_LOG_NOTICE("Treassure ahead!");
    LIB_LOG_INFO("FYI am a log message");
    LIB_LOG_DEBUG("How're you doin today?");

    lib_dump(dummy, strlen(dummy), "memory dump %dB", strlen(dummy));
    lib_hexdump(dummy, strlen(dummy), "memory hexdump %dB", strlen(dummy));

    closelog();

    return 0;
}