#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include <lib_time.h>

static volatile int run = 1;

static void getchar_once() {
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate); //get the terminal state
	ttystate.c_lflag &= ~ICANON; //turn off canonical mode
	ttystate.c_cc[VMIN] = 1; //minimum of number input read.
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); //set the terminal attributes.
}

static void __attribute__((unused)) getchar_line() {
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate); //get the terminal state
	ttystate.c_lflag |= ICANON; //turn on canonical mode
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate); //set the terminal attributes.
}

static void usage() {
    printf("h   This help text\n");
    printf("s   Stop the timer\n");
    printf("r   Resume the timer\n");
    printf("e   Show time elapsed\n");
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
    struct itimerspec it_ela;
    struct itimerspec it_old;
    struct itimerspec it_new;
    timer_t tid;
	int ret;

	ret = lib_timestamp_create(&tid);
    if(ret < 0) {
        fprintf(stderr, "failed to create timer (%d): %s\n",
                errno, strerror(errno));
        goto err;
    }

    ret = lib_timestamp_start(tid);
    if(ret < 0) {
        fprintf(stderr, "failed to start (%d): %s\n",
                errno, strerror(errno));
        goto err_timer_delete;
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
        case 's':
            ret = lib_timestamp_stop(tid, &it_old);
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
        case 'r':
            dump_itimerspec(&it_new, "\rnew");
            ret = lib_timestamp_resume(tid, &it_new);
            if(ret < 0)
                fprintf(stderr, "failed to settime of timer (%d): %s\n",
                        errno, strerror(errno));
            else
                printf("\rtimer resumed\n");
            break;
        case 'e':
            ret = lib_timestamp_elapsed(tid, &it_ela);
            if(ret < 0)
                fprintf(stderr, "failed to gettime (%d): %s\n",
                        errno, strerror(errno));
            else
                dump_itimerspec(&it_ela, "\rela");
            break;
        case 'q':
            run = 0;
            break;
        default:
            break;
        }
    } while(run);

    lib_timestamp_delete(tid);

    return 0;

err_timer_delete:
    lib_timestamp_delete(tid);
err:
    return -1;
}