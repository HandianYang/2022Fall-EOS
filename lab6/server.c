/*
 * lab6/s.c: ATM server
 *
 *  usage: ./server.o <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "sockop.h"

#define SHMSZ 10

/* semaphore */
int semaphore;
#define SEM_MODE 0666
#define SEM_KEY 1122334455
int sem_acquire(int sem);
int sem_release(int sem);

/* socket */
int sockfd, connfd;

/* process */
void childprocess(int connfd);

/* shared memory */
int shmid;

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) {
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Destroy share memory segment */
    int retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0)
        errexit("Error: shmctl(IPC_RMID)\n");

    /* Remove semaphore */
    if (semctl (semaphore, 0, IPC_RMID, 0) < 0)
        errexit("Error: unable to remove semaphore %d\n", SEM_KEY);
}
/* zombie process handler */
void zombie_process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* main function */
int main(int argc, char *argv[])
{
    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);

    if (signal(SIGINT, interrupt_handler) == SIG_ERR)
        fprintf(stderr, "Failed to caught SIGINT signal\n");
    if (signal(SIGCHLD, zombie_process_handler) == SIG_ERR)
        fprintf(stderr, "Failed to caught SIGCHLD signal\n");

    /* Create semaphore */
    semaphore = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaphore < 0) 
        errexit("Error: unable to create semaphore %d\n", SEM_KEY);

    if (semctl(semaphore, 0, SETVAL, 1) < 0)
        errexit("Error: unable to set initial value 1 to semaphore %d\n", SEM_KEY);
    
    /* Create the shared memory segment */
    key_t key = 5678;
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0)
        errexit("Error: shmget()\n");
    
    char *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)
        errexit("Error: shmat()\n");
    memset(shm, '0', SHMSZ);
    shmdt(shm);

    /* Create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    /* Others */
    int retval;         // return value for checking validity of functions

    /* Keep accept client connection */
    pid_t childpid;
    while (1) {
        /* Accept the connection of some client */
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            fprintf(stderr, "Error: accept()\n");
            goto F_EXIT;
        }
        
        if((childpid = fork()) == 0) {
            // child
            childprocess(connfd);            
        } else if (childpid > 0) {
            // parent

        } else {
            // error
            errexit("Error: fork()\n");
        }
        
    }

F_EXIT:
    waitpid(childpid, NULL, 0);

    return 0;
}


/* Do operations */
void childprocess(int connfd) {
    char rcv[BUF_SIZE];
    int rcv_length;

    /* Read from the client */
    memset(rcv, 0, BUF_SIZE);
    if ((rcv_length = read(connfd, rcv, BUF_SIZE)) == -1)
        errexit("Error: read()\n");
    rcv[rcv_length] = '\0';

    /* Parse the received message */
#ifdef DEBUG
    printf("[DEBUG]\n (Child) Parsing the string...\n");
#endif
    char *p = strtok(rcv, " \n");
    char operation[BUF_SIZE];
    int money, times;
    unsigned short token_index = 0;
    while (p != NULL) {
#ifdef DEBUG
        printf("\t\ttoken: %s\n", p);
#endif
        switch (token_index++) {
        case 0:
            strcpy(operation, p);
            break;
        case 1:
            money = atoi(p);
            break;
        case 2:
            times = atoi(p);
        default:
            break;
        }
            
        p = strtok(NULL, " \n");
    }

#ifdef DEBUG
    printf("[DEBUG]\n (Child) The operation from user:\n");
    printf("\t\tOperation: %s\t\tMoney: %d\n\n", operation, money);
#endif
    unsigned short i;
    for (i = 0; i < times; i++) {
        /* Acquire semaphore */
        sem_acquire(semaphore);

        /* Attach the shared memory segment */
        char *shm;
        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)
            errexit("Error: shmat()\n");
        
        /* Obtain current bank savings */
        int savings = atoi(shm);

        /* Do deposit or withdraw */
        if (strcmp(operation, "deposit") == 0)
            savings += money;
        else if (strcmp(operation, "withdraw") == 0)
            savings -= money;

        printf("After %s %d dollar(s), the savings now is %d\n", operation, money, savings);

        /* Store new bank savings back to shm */
        memset(shm, '0', SHMSZ);
        
        int sign = 1;
        if (savings < 0) {
            sign = -1;
            savings *= sign;
        }
        
        unsigned short index = SHMSZ;
        while (savings) {
            shm[--index] = savings % 10 + '0';
            savings /= 10;
        }
        if (sign == -1)
            shm[0] = '-';   // negative number
        
        /* Detach the shared memory segment */
        shmdt(shm);

        /* Release semaphore */
        sem_release(semaphore);
    }
    
    close(connfd);
    exit(0);
}


int sem_acquire(int sem) {
    struct sembuf sop;  /* the operation parameters */
    sop.sem_num = 0;    /* access the 1st (and only) sem in the array */
    sop.sem_op = -1;    /* wait..*/
    sop.sem_flg = 0;    /* no special options needed */

    if (semop (sem, &sop, 1) < 0)
        errexit("Error: sem_acquire() failed\n");
    
    return 0;
}

int sem_release(int sem) {
    struct sembuf sop;  /* the operation parameters */
    sop.sem_num = 0;    /* the 1st (and only) sem in the array */
    sop.sem_op = 1;     /* signal */
    sop.sem_flg = 0;    /* no special options needed */

    if (semop(sem, &sop, 1) < 0)
        errexit("Error: sem_release() failed\n");
    
    return 0;
}