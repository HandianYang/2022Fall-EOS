#ifndef _IPC_LIB_H_
#define _IPC_LIB_H_

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include "linked_list.h"

/************************* semaphore *************************/
#define SEM_MODE 0666
#define SEM_SYS_KEY 1000
#define SEM_BUF_KEY 3000
int semaCDCSystem;
int semaBus1Result, semaBus2Result;

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
#define NUM_REGION 9
#define SHM_SYS_KEY 10000
#define SHM_BUF_KEY 30000
int shmSevereCaseID, shmMildCaseID;     // sizeof(int) * REGION_NUM
int shmBus1ResultID, shmBus2ResultID;   // sizeof(struct List)

/* Initialization for shared memory
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int sharedMemoryInit();

/* Remove all shared memory
 * 
 * [return]
 *  0 : success
 * -1 : failure
 */
int sharedMemoryRemove();



/************************* socket *************************/
int sockfd, connfd;

#endif