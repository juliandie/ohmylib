#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include <timestamp.h>

static volatile int run = 1;

static void getchar_once() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate); /**< get terminal attr */
    ttystate.c_lflag &= ~ICANON; /**< turn off canonical mode */
    ttystate.c_lflag &= ~ECHO; /**< disable echo */
    ttystate.c_cc[VMIN] = 1; /**< minimum number of inputs */
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); /**< set terminal attr */
}

static void __attribute__((unused)) termios_reset() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate); //get the terminal state
    ttystate.c_lflag |= ICANON; //turn on canonical mode
    ttystate.c_lflag |= ECHO; /**< enable echo */
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); //set the terminal attributes.
}

static void usage() {
    printf("h   show [h]elp text\n"
           "e   show time [e]lapsed\n"
           "r   [r]eset timestamp timer\n"
           "q   [q]uit this program\n");
}

static void dump_itimerspec(struct itimerspec *t, const char *prefix) {
    printf("%s: it_interval = { .tv_sec = %3lu, .tv_nsec = %9lu }, "
           "it_value = { .tv_sec = %3lu, .tv_nsec = %9lu }\n",
           prefix,
           t->it_interval.tv_sec, t->it_interval.tv_nsec,
           t->it_value.tv_sec, t->it_value.tv_nsec);
}

int main() {
    timer_t tid;
    int ret;

    ret = timestamp_create(&tid);
    if(ret < 0) {
        fprintf(stderr, "failed to create timer: %s\n", strerror(errno));
        return -1;
    }

    // modify tty that fgetc reacts on any keypress
    getchar_once();
    usage();

    do {
        int c;
        c = fgetc(stdin); // don't use getchar tho!
        switch(c) {
        case 'h':
            printf("\r");
            usage();
            break;
        case 'e':
        {
            struct itimerspec elapsed_value;
            ret = timestamp_elapsed(tid, &elapsed_value);
            if(ret < 0) {
                fprintf(stderr, "failed to gettime: %s\n", strerror(errno));
                continue;
            }
            dump_itimerspec(&elapsed_value, "elapsed");
        } break;
        case 'r':
        {
            struct itimerspec elapsed_value;
            if(timestamp_reset(tid) < 0) {
                continue;
            }
            ret = timestamp_elapsed(tid, &elapsed_value);
            if(ret < 0) {
                fprintf(stderr, "failed to gettime: %s\n", strerror(errno));
                continue;
            }
            dump_itimerspec(&elapsed_value, "elapsed");
        } break;
        case 'q':
            run = 0;
            break;
        default:
            break;
        }
    } while(run);

    timestamp_destory(tid);
    termios_reset();
    return 0;
}