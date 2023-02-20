/*
 * lab6/client.c: ATM clients
 *
 *  usage: ./client.o <ip> <port> <deposit/withdraw> <amount> <times>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"

int connfd;

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) 
{
    close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc != 6)
        errexit("Usage: %s <ip> <port> <deposit/withdraw> <amount> <times>\n", argv[0]);
    
    // if (strcmp(argv[3], "deposit") && strcmp(argv[3], "withdraw"))
    //     errexit("InputFormatError: argv[3] should be either \'deposit\' or \'withdraw\'\n");
    // if (atoi(argv[4]) < 0)
    //     errexit("");
        
    
    signal(SIGINT, interrupt_handler);

    char snd[BUF_SIZE];

    /* create socket and connect to server */
    connfd = connectsock(argv[1], argv[2], "tcp");

    /* Write user command to the server */
    sprintf(snd, "%s %s %s\n", argv[3], argv[4], argv[5]);
    if (write(connfd, snd, strlen(snd)+1) == -1)
        goto err_write;

#ifdef DEBUG
    printf("[DEBUG]\tClient sends: %s\n", snd);
#endif

    /* close client socket */
    close(connfd);

    return 0;


err_write:
    fprintf(stderr, "Error: write()\n");
    goto err_exit;
err_read:
    fprintf(stderr, "Error: read()\n");
    goto err_exit;

err_exit:
    close(connfd);
    return -1;
}