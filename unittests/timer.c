#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

static volatile int run = 1;
#if 0
static void sig_reset(int signum) {
    signal(signum, SIG_DFL);
}
#endif

static int sig_register(int signum, void (*handler)(int)) {
    struct sigaction act = {
        .sa_flags = SA_RESTART,
        .sa_handler = handler,
    };
    sigemptyset(&act.sa_mask);
    return sigaction(signum, &act, NULL);
}

static void alrm_handler(int signum) {
    (void)signum;
    printf("a timer has expired...\n");
}

static void getchar_once() {
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
    //turn off canonical mode
    ttystate.c_lflag &= ~ICANON;
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

static void __attribute__((unused)) getchar_line() {
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
    //turn on canonical mode
    ttystate.c_lflag |= ICANON;
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

static void usage() {
    printf("h   This help text\n");
    printf("p   Pause the timer\n");
    printf("r   Resume the timer\n");
    printf("e   Show time elapsed\n");
    printf("x   Show if the timer has been expired\n");
    printf("q   Quit this program\n");
}

static void dump_itimerspec(struct itimerspec *t, const char *pre) {
    printf("%s: it_interval = { .tv_sec = %3lu, .tv_nsec = %9lu }, "
           "it_value = { .tv_sec = %3lu, .tv_nsec = %9lu }\n",
           pre,
           t->it_interval.tv_sec, t->it_interval.tv_nsec,
           t->it_value.tv_sec, t->it_value.tv_nsec);
}

int main() {
    timer_t tid;
    struct sigevent ev = {
        .sigev_signo = SIGALRM,
        .sigev_notify = SIGEV_SIGNAL,
    };
    struct itimerspec it_new = {
        .it_interval = {
            .tv_sec = 5,
            .tv_nsec = 0,
        },
        .it_value = {
            .tv_sec = 5,
            .tv_nsec = 0,
        }
    };
    struct itimerspec it_old = {};
    struct itimerspec it_rem = {};
    int ret;

    sig_register(SIGALRM, &alrm_handler);

    ret = timer_create(CLOCK_REALTIME, &ev, &tid);
    if(ret < 0) {
        fprintf(stderr, "failed to create timer (%d): %s\n",
                errno, strerror(errno));
        goto err;
    }

    ret = timer_settime(tid, 0, &it_new, NULL);
    if(ret < 0) {
        fprintf(stderr, "failed to settime of timer (%d): %s\n",
                errno, strerror(errno));
        goto err_timer_delete;
    }

    // modify tty that fgetc reacts on any keypress
    getchar_once();
    usage();

    do {
        int c = fgetc(stdin); // don't use getchar tho!

        switch(c) {
        case 'h':
            printf("\r");
            usage();
            break;
        case 'p': // pause
            memset(&it_rem, 0, sizeof(struct itimerspec));
            ret = timer_settime(tid, 0, &it_rem, &it_old);
            dump_itimerspec(&it_old, "\rold");
            if(ret < 0) {
                fprintf(stderr, "failed to stop timer (%d): %s\n",
                        errno, strerror(errno));
            }
            else {
                printf("\rtimer stopped\n");
                if(it_old.it_value.tv_sec != 0)
                    memcpy(&it_new, &it_old, sizeof(struct itimerspec));
            }
            break;
        case 'r': // resume
            dump_itimerspec(&it_new, "\rnew");
            ret = timer_settime(tid, 0, &it_new, NULL);
            if(ret < 0)
                fprintf(stderr, "failed to settime of timer (%d): %s\n",
                        errno, strerror(errno));
            else
                printf("\rtimer resumed\n");
            break;
        case 'e': // remaining
            ret = timer_gettime(tid, &it_rem);
            if(ret < 0)
                fprintf(stderr, "failed to gettime (%d): %s\n",
                        errno, strerror(errno));
            else
                dump_itimerspec(&it_rem, "\rrem");
            break;
        case 'x': // expired
            ret = timer_getoverrun(tid);
            if(ret < 0) 
                fprintf(stderr, "failed to get overrun (%d): %s\n",
                        errno, strerror(errno));
            else
                printf("\rtimer has expired (%d)\n", ret);
            break;
        case 'q':
            run = 0;
            break;
        default:
            break;
        }
    } while(run);

    timer_delete(tid);

    return 0;

err_timer_delete:
    timer_delete(tid);
err:
    return -1;
}