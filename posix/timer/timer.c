#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

/** timer without event, expires after 5 seconds
#include <time.h>
#include <signal.h>
int main() {
    timer_t tid;
    struct sigevent ev_sig = {
        .sigev_notify = SIGEV_NONE,
    };
    // used to stop timer
    struct itimerspec stop_tim = {};
    // used to store timer values
    struct itimerspec old_tim = {};
    struct itimerspec new_tim = {
		.it_value = { .tv_sec = 5, .tv_nsec = 0, }
    };
    int ret;
    //...
    // create a timer - See man 2 timer_create
    ret = timer_create(CLOCK_REALTIME, &ev_sig, &tid);
    if(ret < 0) return -1; //... error
    //...
    // start the timer
    ret = timer_settime(tid, 0, &new_tim, NULL);
    if(ret < 0) return -1; //... error
    //...
    // check if timer has expired
    ret = timer_gettime(tid, &old_tim);
    if(ret < 0) return -1; //... error
    else if(ret == 0) {}//... not expired
    else {} // (ret > 0) //... expired
    //...
    ret = timer_settime(tid, 0, &stop_tim, &old_tim);
    if(ret < 0) return -1; //... error
    //...
    // free resources
    timer_delete(tid);
}
 */

/** timer with event, repeated every 3 seconds
#include <time.h>
#include <signal.h>
int main() {
    timer_t tid;
    struct sigevent ev_alsig = {
    // Use SIGALRM as signal when the timer has expired
        .sigev_signo = SIGALRM,
        .sigev_notify = SIGEV_SIGNAL,
    };
    // used to store timer values
    struct itimerspec old_tim = {};
    struct itimerspec new_tim = {
        // autoreload after expire
		.it_interval = { .tv_sec = 3, .tv_nsec = 0, },
		.it_value = { .tv_sec = 3, .tv_nsec = 0, }
    };
    int ret;

    // create a timer - See man 2 timer_create
    ret = timer_create(CLOCK_REALTIME, &ev_alsig, &tid);
    if(ret < 0) return -1;//... error
    //...
    // start the timer
    ret = timer_settime(tid, 0, &new_tim, NULL);
    if(ret < 0) return -1; //... error
    //...
    ret = timer_gettime(tid, &old_tim);
    if(ret < 0) return -1; //... error
    //...
    // free resources
    timer_delete(tid);
}
*/

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
    tcgetattr(STDIN_FILENO, &ttystate); /**< get terminal attr */
    ttystate.c_lflag &= ~ICANON; /**< disable canonical mode */
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
           "p   [p]ause the timer\n"
           "r   [r]esume the timer\n"
           "e   show time [e]lapsed\n"
           "x   show if timer has e[x]pired\n"
           "i   setup [i]nterval mode\n"
           "o   setup [o]neshot mode\n"
           "q   [q]uit this program\n");
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
    struct itimerspec default_value = {
        .it_interval = {
            .tv_sec = 5,
            .tv_nsec = 0,
        },
        .it_value = {
            .tv_sec = 5,
            .tv_nsec = 0,
        }
    };
    struct itimerspec stored_value;
    int ret;

    sig_register(SIGALRM, &alrm_handler);

    ret = timer_create(CLOCK_REALTIME, &ev, &tid);
    if(ret < 0) {
        fprintf(stderr, "failed to create timer: %s\n", strerror(errno));
        goto err;
    }

    ret = timer_settime(tid, 0, &default_value, NULL);
    if(ret < 0) {
        fprintf(stderr, "failed to settime of timer: %s\n", strerror(errno));
        goto err_timer_delete;
    }

    // modify tty that fgetc reacts on any keypress
    getchar_once();
    usage();

    do {
        int c = fgetc(stdin); // don't use getchar tho!

        switch(c) {
        case 'h':
            usage();
            break;
        case 'p':
        { // pause
            struct itimerspec new_value = {0};
            struct itimerspec curr_value
            ret = timer_settime(tid, 0, &new_value, &curr_value);
            if(ret < 0) {
                fprintf(stderr, "failed to stop timer: %s\n", strerror(errno));
                continue;
            }

            if(curr_value.it_value.tv_sec != 0 ||
               curr_value.it_value.tv_nsec != 0) {
                dump_itimerspec(&curr_value, "paused");
                memcpy(&stored_value, &curr_value, sizeof(stored_value));
            }
        } break;
        case 'r':
        { // resume
            if(stored_value.it_value.tv_sec == 0 &&
               stored_value.it_value.tv_nsec == 0) {
                continue;
            }
            ret = timer_settime(tid, 0, &stored_value, NULL);
            if(ret < 0) {
                fprintf(stderr, "failed to settime of timer: %s\n", strerror(errno));
                continue;
            }

            dump_itimerspec(&stored_value, "resumed");
            memset(&stored_value, 0, sizeof(stored_value));
        } break;
        case 'e':
        { // remaining
            struct itimerspec curr_value = {0};
            ret = timer_gettime(tid, &curr_value);
            if(ret < 0) {
                fprintf(stderr, "failed to gettime: %s\n", strerror(errno));
                continue;
            }

            dump_itimerspec(&curr_value, "remaining");
        } break;
        case 'x':
        { // expired
            ret = timer_getoverrun(tid);
            if(ret < 0) {
                fprintf(stderr, "failed to get overrun: %s\n", strerror(errno));
                continue;
            }

            printf("\rtimer has expired (%d)\n", ret);
        } break;
        case 'i':
        { // intervall
            ret = timer_settime(tid, 0, &default_value, NULL);
            if(ret < 0) {
                fprintf(stderr, "failed to settime of timer: %s\n", strerror(errno));
                continue;
            }

            dump_itimerspec(&default_value, "invervall");
        } break;
        case 'o':
        { // oneshot
            struct itimerspec once_value = {
                .it_value = {
                    .tv_sec = 5,
                    .tv_nsec = 0,
                }
            };
            ret = timer_settime(tid, 0, &once_value, NULL);
            if(ret < 0) {
                fprintf(stderr, "failed to settime of timer: %s\n", strerror(errno));
                continue;
            }

            dump_itimerspec(&once_value, "oneshot");
        } break;
        case 'q':
            run = 0;
            break;
        default:
            break;
        }
    } while(run);

    timer_delete(tid);
    termios_reset();
    return 0;

err_timer_delete:
    timer_delete(tid);
err:
    termios_reset();
    return -1;
}