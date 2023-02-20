#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_MODE 0666

typedef struct {
    int guess;
    char result[8];
} Data;

sig_atomic_t begin_num = 0;
sig_atomic_t end_num;
sig_atomic_t shmid;
sig_atomic_t gamer_pid;

void timer_handler(int signum);

int main(int argc, char **argv) {
    
    if (argc != 4) {
        fprintf(stderr, "[Error] Usage: %s <shm_key> <upper_bound> <gamer_pid>", argv[0]);
        exit(-1);
    }
    int shm_key = atoi(argv[1]);
    end_num = atoi(argv[2]);
    gamer_pid = atoi(argv[3]);

    /* Create shared memory segment */
    if ((shmid = shmget(shm_key, sizeof(Data), IPC_CREAT | SHM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget()\n");
        exit(-1);
    }

    struct sigaction sa;
    struct itimerval timer;
    struct timespec start, finish;
    double elapsed_time;
    int retval;

    /* Install timer_handler as the signal handler for SIGVTALRM */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    
    /* Reset the timer back to 250 msec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);

    /* sleep 10 sec */
    clock_gettime(CLOCK_REALTIME, &start);
    do {
        clock_gettime(CLOCK_REALTIME, &finish);
        elapsed_time = finish.tv_sec - start.tv_sec;
        elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1e09;
    } while (elapsed_time < 10);

    return 0;
}

void timer_handler(int signum) {
    Data *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "Error: shmat()\n");
        return;
    }
    

    if (strcmp(shm->result, "bigger") == 0)
        begin_num = shm->guess;
    else if (strcmp(shm->result, "smaller") == 0)
        end_num = shm->guess;
    else if (strcmp(shm->result, "bingo") == 0)
        return;

    shm->guess = (begin_num + end_num) / 2;
    
    printf("[game] Guess: %d\n", shm->guess);

    shmdt(shm);

    kill(gamer_pid, SIGUSR1);
}
