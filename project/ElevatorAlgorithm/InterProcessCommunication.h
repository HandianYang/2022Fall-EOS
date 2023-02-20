#ifndef _INTER_PROCESS_COMMUNICATION_H_
#define _INTER_PROCESS_COMMUNICATION_H_

#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

#include "ElevatorProperty.h"

/************************* semaphore *************************/
#define SEM_MODE 0666
#define SEM_ELEVATOR_KEY 0x1000
#define SEM_FLOOR_KEY 0x2000
#define SEM_PASSENGER_ID_KEY 0x3000
#define SEM_INT_WHICH_ELEVATOR_KEY 0x4000
#define SEM_PID 0x5000

extern int semaphoreElevator[NUM_OF_ELEVATOR];
extern int semaphoreFloor[NUM_OF_FLOOR];
extern int semaphorePassengerID;
extern int semaphoreIntWhichElevator; // [e]'0104
extern int semaphorePid;

/* Initialization for semaphores
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int semaphoreInit();

/* Remove all semaphores
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int semaphoreRemove();

/* Acquire the given semaphore */
int sem_acquire(int sem);

/* Release the given semaphore */
int sem_release(int sem);



/************************* shared memory *************************/
#define SHM_ELEVATOR_KEY 0x10000
#define SHM_FLOOR_KEY 0x20000
#define SHM_PASSENGER_ID_KEY 0x30000
#define SHM_INT_WHICH_ELEVATOR 0x40000 // [e]'0104
#define SHM_PID 0x50000

extern int shmElevatorID[NUM_OF_ELEVATOR];     // sizeof(Elevator)
extern int shmFloorID[NUM_OF_FLOOR];           // sizeof(Floor)
extern int shmPassengerID_ID;                  // sizeof(unsigned long)
extern int shmIntWhichElevatorID;              // sizeof(int)*2
extern int shmPidID;                           // sizeof(int)*NUM_OF_ELEVATOR*3

/* Initialization for shared memory
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int sharedMemoryInit(int algorithm);

/* Remove all shared memory
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int sharedMemoryRemove();



/************************* socket *************************/

extern int sockfd;
extern int connfd;

#define errexit(format, arg ...) exit(printf(format, ##arg))

/* 
* passivesock() - allocate & bind a server socket using TCP or UDP
 * 
 * Arguments:
 *  service     - service associated with the desired port
 *  transport   - transport protocol to use ("tcp" or "udp")
 *  qlen        - maximum server request queue length
 */
int passivesock(const char *service, const char *transport, int qlen);

/* 
 * connectsock()- allocate & connect a socket using TCP or UDP
 * 
 * Arguments:
 *  host        - name of host to which connection is desired
 *  service     - service associated with the desired port
 *  transport   - name of transport protocol to use ("tcp" or "udp")
 */
int connectsock(const char *host, const char *service, const char *transport);


/************************* signal *************************/

/* ctrl-c signal handler */
void SIGINT_handler(int signum);

/* zombie process handler */
void SIGCHLD_handler(int signum);



#endif