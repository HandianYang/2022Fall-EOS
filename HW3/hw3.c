/*
 * hw3/hw3.c: CDC system server (multiple connection)
 *
 *  usage: ./server.o <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"
#include "IPC_lib.h"
#include "process_lib.h"

/* main function */
int main(int argc, char *argv[])
{
    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);
    
    printf("****************************\n");
    printf("*      DEBUG mode: %s     *\n", DEBUG ? "ON " : "OFF");
    printf("****************************\n");    

    /* Register important signals */
    if (signal(SIGINT, interrupt_handler) == SIG_ERR)
        fprintf(stderr, "Failed to caught SIGINT signal\n");
    if (signal(SIGCHLD, zombie_process_handler) == SIG_ERR)
        fprintf(stderr, "Failed to caught SIGCHLD signal\n");


    int retval;

    /* Initialize semaphores */
    retval = semaphoreInit();
    if (retval < 0) 
        errexit("Error: Failed to initialize all semaphores\n");

    /* Initialize shared memory */
    retval = sharedMemoryInit();
    if (retval < 0) 
        errexit("Error: Failed to initialize all shared memory\n");
    

    /* Create ambulance processes */
    pid_t ambulance_pid[2];

    if ((ambulance_pid[0] = fork()) == 0) 
        ambulanceProcess(1);
    else if (ambulance_pid[0] < 0)
        errexit("Error: Failed to create process of ambulance 1\n");
    
    if ((ambulance_pid[1] = fork()) == 0) 
        ambulanceProcess(2);
    else if (ambulance_pid[1] < 0)
        errexit("Error: Failed to create process of ambulance 2\n");


    /* Create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    /* Keep accept client connection */
    pid_t client_pid;
    while (1) {
        /* Accept the connection of some client */
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            fprintf(stderr, "Error: accept()\n");
            goto F_EXIT;
        }
        printf("Connection has been established!!\n");
        
        if((client_pid = fork()) == 0) {
            // child
            clientProcess(connfd);            
        } else if (client_pid < 0) {
            // error
            fprintf(stderr, "Error: fork()\n");
            goto F_EXIT;
        }
        
    }

F_EXIT:
    waitpid(client_pid, NULL, 0);
    waitpid(ambulance_pid[0], NULL, 0);
    waitpid(ambulance_pid[1], NULL, 0);
    
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Destroy share memory segment */
    retval = sharedMemoryRemove();
    if (retval < 0)
        fprintf(stderr, "Error: Failed to remove all shared memory\n");
    
    /* Remove semaphores */
    retval = semaphoreRemove();
    if (retval < 0)
        fprintf(stderr, "Error: Failed to remove all semaphores\n");

    return 0;
}
