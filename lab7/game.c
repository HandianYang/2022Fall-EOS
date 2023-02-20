#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/shm.h>

#define SHM_MODE 0666

typedef struct {
    int guess;
    char result[8];
} Data;

sig_atomic_t shmid;
sig_atomic_t answer;

void sigusr1_handler(int signum);

void ctrl_c_handler(int signum);

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "[Error] Usage: %s <shm_key> <answer>", argv[0]);
        exit(-1);
    }
    int shm_key = atoi(argv[1]);
    answer = atoi(argv[2]);

    /* Create shared memory segment */
    if ((shmid = shmget(shm_key, sizeof(Data), IPC_CREAT | SHM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget()\n");
        exit(-1);
    }

    /* Register important signals */
    if (signal(SIGINT, ctrl_c_handler) == SIG_ERR)
        fprintf(stderr, "Failed to caught SIGINT signal\n");
    
    /* register handler to SIGUSR1 */
    struct sigaction sa;

    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = sigusr1_handler;
    sigaction (SIGUSR1, &sa, NULL);

    printf("[game] Game PID: %d\n", getpid());

    while (1);

    return 0;
}

void sigusr1_handler(int signum) {
    Data *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "Error: shmat()\n");
        return;
    }

    if (shm->guess == answer) {
        strcpy(shm->result, "bingo");
    } else if (shm->guess < answer) {
        strcpy(shm->result, "bigger");
    } else if (shm->guess > answer) {
        strcpy(shm->result, "smaller");
    }
    printf("[game] Guess %d, %s\n", shm->guess, shm->result);

    shmdt(shm);
}

void ctrl_c_handler(int signum) {
    /* Destroy share memory segment */
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: shmctl()\n");
    }

    exit(0);
}