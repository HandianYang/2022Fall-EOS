#include "InterProcessCommunication.h"

/************************* semaphore *************************/

int semaphoreElevator[NUM_OF_ELEVATOR];
int semaphoreFloor[NUM_OF_FLOOR];
int semaphorePassengerID;
int semaphoreIntWhichElevator;
int semaphorePid;

int semaphoreInit() {
    int i, retval = 0;
    int semaphore_elevator_key = SEM_ELEVATOR_KEY;
    int semaphore_floor_key = SEM_FLOOR_KEY;

    // semaphores for Elevator
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        semaphoreElevator[i] = semget(semaphore_elevator_key, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
        if (semaphoreElevator[i] < 0) {
            fprintf(stderr, "Error: unable to create semaphore %d\n", semaphore_elevator_key);
            retval = -1;
        }
        if (semctl(semaphoreElevator[i], 0, SETVAL, 1) < 0) {
            fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphore_elevator_key);
            retval = -1;
        }
        semaphore_elevator_key++;
    }

    // semaphores for Floor
    for (i = 0; i < NUM_OF_FLOOR; i++) {
        semaphoreFloor[i] = semget(semaphore_floor_key, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
        if (semaphoreFloor[i] < 0) {
            fprintf(stderr, "Error: unable to create semaphore %d\n", semaphore_floor_key);
            retval = -1;
        }
        if (semctl(semaphoreFloor[i], 0, SETVAL, 1) < 0) {
            fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphore_floor_key);
            retval = -1;
        }
        semaphore_floor_key++;
    }

    // semaphore for ID of Passengers
    semaphorePassengerID = semget(SEM_PASSENGER_ID_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaphorePassengerID < 0) {
        fprintf(stderr, "Error: unable to create semaphore %d\n", SEM_PASSENGER_ID_KEY);
        retval = -1;
    }
    if (semctl(semaphorePassengerID, 0, SETVAL, 1) < 0) {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", SEM_PASSENGER_ID_KEY);
        retval = -1;
    }

    // semaphore for interrupt
    semaphoreIntWhichElevator = semget(SEM_INT_WHICH_ELEVATOR_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaphoreIntWhichElevator < 0)
    {
        fprintf(stderr, "Error: unable to create semaphore %d\n", semaphoreIntWhichElevator);
        return -1;
    }
    if (semctl(semaphoreIntWhichElevator, 0, SETVAL, 1) < 0)
    {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphoreIntWhichElevator);
        return -1;
    }

    // semaphore for process pid
    semaphorePid = semget(SEM_PID, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaphorePid < 0)
    {
        fprintf(stderr, "Error: unable to create semaphore %d\n", semaphorePid);
        return -1;
    }
    if (semctl(semaphorePid, 0, SETVAL, 1) < 0)
    {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphorePid);
        return -1;
    }

    return retval;
}


int semaphoreRemove() {
    int i, retval = 0;
    int semaphore_elevator_key = SEM_ELEVATOR_KEY;
    int semaphore_floor_key = SEM_FLOOR_KEY;

    // semaphores for Elevator
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        if (semctl (semaphoreElevator[i], 0, IPC_RMID, 0) < 0) {
            fprintf(stderr, "  Error: unable to remove semaphore %d\n\n", semaphore_elevator_key);
            retval = -1;
        }
        semaphore_elevator_key++;
    }

    // semaphores for Floor
    for (i = 0; i < NUM_OF_FLOOR; i++) {
        if (semctl (semaphoreFloor[i], 0, IPC_RMID, 0) < 0) {
            fprintf(stderr, "  Error: unable to remove semaphore %d\n\n", semaphore_floor_key);
            retval = -1;
        }
        semaphore_floor_key++;
    }

    // semaphore for ID of Passengers
    if (semctl (semaphorePassengerID, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "  Error: unable to remove semaphore %d\n\n", SEM_PASSENGER_ID_KEY);
        retval = -1;
    }

    // semaphores for interrupt
    if (semctl(semaphoreIntWhichElevator, 0, IPC_RMID, 0) < 0)
    {
        fprintf(stderr, "Error: unable to remove semaphore %d\n", SEM_INT_WHICH_ELEVATOR_KEY);
        return -1;
    }

    // semaphore for process pid
    if (semctl (semaphorePid, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "  Error: unable to remove semaphore %d\n\n", SEM_PID);
        retval = -1;
    }

    return retval;
}


int sem_acquire(int sem) {
    struct sembuf sop;  /* the operation parameters */
    sop.sem_num = 0;    /* access the 1st (and only) sem in the array */
    sop.sem_op = -1;    /* wait..*/
    sop.sem_flg = 0;    /* no special options needed */

    if (semop (sem, &sop, 1) < 0) {
        // fprintf(stderr, "Error: sem_acquire() failed\n");
        return -1;
    }
    
    return 0;
}

int sem_release(int sem) {
    struct sembuf sop;  /* the operation parameters */
    sop.sem_num = 0;    /* the 1st (and only) sem in the array */
    sop.sem_op = 1;     /* signal */
    sop.sem_flg = 0;    /* no special options needed */

    if (semop(sem, &sop, 1) < 0) {
        // fprintf(stderr, "Error: sem_release() failed\n");
        return -1;
    }
    
    return 0;
}


/************************* shared memory *************************/

int shmElevatorID[NUM_OF_ELEVATOR];     // sizeof(Elevator)
int shmFloorID[NUM_OF_FLOOR];           // sizeof(Floor)
int shmPassengerID_ID;                  // sizeof(unsigned long)
int shmIntWhichElevatorID;
int shmPidID;

int sharedMemoryInit(int algorithm) {
    int i, retval = 0;
    Elevator *elevator;
    Floor *floor;
    unsigned long *shm_id;
    int *shm_int;
    int *shm_pid;

    // shared memory for Elevator
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        if ((shmElevatorID[i] = shmget(SHM_ELEVATOR_KEY+i, sizeof(Elevator), IPC_CREAT | SEM_MODE)) < 0) {
            fprintf(stderr, "  Error: shmget(elevator[%d])\n\n", i);
            retval = -1;
        }

        if ((elevator = shmat(shmElevatorID[i], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "  Error: shmat(elevator[%d])\n", i);
            retval = -1;
        }

        // default settings for elevators
        elevator->ID = i;

        elevator->callSequence.length = 0;
        elevator->floorCalls.length = 0;
        elevator->liftCalls.length = 0;

        elevator->load = 0;
        elevator->algorithm = algorithm;
        elevator->current_floor = 1;
        elevator->direction = 0;
        elevator->idle = 1;
        elevator->abnormal = 0;
        
        elevator->passenger_loading_time = 1;
        elevator->passenger_unloading_time = 1;
        elevator->velocity = 1;
        elevator->capacity = 25;
        shmdt(elevator);
    }

    // shared memory for Floor
    for (i = 0; i < NUM_OF_FLOOR; i++) {
        if ((shmFloorID[i] = shmget(SHM_FLOOR_KEY+i, sizeof(Floor), IPC_CREAT | SEM_MODE)) < 0) {
            fprintf(stderr, "  Error: shmget(floor[%d])\n\n", i);
            retval = -1;
        }
        if ((floor = shmat(shmFloorID[i], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "  Error: shmat(floor[%d])\n\n", i);
            retval = -1;
        }

        // default settings for floors
        floor->ID = i+1;
        floor->passengers.length = 0;
        floor->passengers_unchecked.length = 0;
        shmdt(floor);
    }

    // shared memory segment for ID of Passengers
    if ((shmPassengerID_ID = shmget(SHM_PASSENGER_ID_KEY, sizeof(unsigned long), IPC_CREAT | SEM_MODE)) < 0) {
        fprintf(stderr, "  Error: shmget(passengerID)\n\n");
        retval = -1;
    }

    if ((shm_id = shmat(shmPassengerID_ID, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "  Error: shmat(passengerID)\n\n");
        retval = -1;
    }
    // initial value = 1
    *shm_id = 1;
    shmdt(shm_id);

    // shared memory for interrupt //[e]'0104
    if ((shmIntWhichElevatorID = shmget(SEM_INT_WHICH_ELEVATOR_KEY, sizeof(int), IPC_CREAT | SEM_MODE)) < 0)
    {
        fprintf(stderr, "Error: shmget(shmInterruptID)\n");
        return -1;
    }
    if ((shm_int = shmat(shmIntWhichElevatorID, NULL, 0)) == (void *)-1)
    {
        fprintf(stderr, "Error: shmat(shmInterruptID)\n");
        return -1;
    }
    *shm_int = 9;
    shmdt(shm_int);

    // shared memory for process pid
    if ((shmPidID = shmget(SEM_PID, NUM_OF_ELEVATOR*3*sizeof(int), IPC_CREAT | SEM_MODE)) < 0)
    {
        fprintf(stderr, "Error: shmget(shmInterruptID)\n");
        return -1;
    }
    if ((shm_pid = shmat(shmPidID, NULL, 0)) == (void *)-1)
    {
        fprintf(stderr, "Error: shmat(shmInterruptID)\n");
        return -1;
    }
    memset(shm_pid, 0, NUM_OF_ELEVATOR*3);
    shmdt(shm_pid);

    return retval;
}


int sharedMemoryRemove() {
    int i, retval = 0;

    // shared memory for Elevator
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        if (shmctl(shmElevatorID[i], IPC_RMID, NULL) < 0) {
            fprintf(stderr, "  Error: shmctl(elevator[%d])\n\n", i);
            retval = -1;
        }
    }

    // shared memory for Floor
    for (i = 0; i < NUM_OF_FLOOR; i++) {
        if (shmctl(shmFloorID[i], IPC_RMID, NULL) < 0) {
            fprintf(stderr, "  Error: shmctl(floor[%d])\n\n", i);
            retval = -1;
        }
    }

    // shared memory for ID of Passengers
    if (shmctl(shmPassengerID_ID, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "  Error: shmctl(passengerID)\n\n");
        retval = -1;
    }

    // shared memory for interrupt
    if (shmctl(shmIntWhichElevatorID, IPC_RMID, NULL) < 0)
    {
        fprintf(stderr, "Error: shmctl(shmIntWhichElevatorID)\n");
        return -1;
    }

    // shared memory for process pid
    if (shmctl(shmPidID, IPC_RMID, NULL) < 0)
    {
        fprintf(stderr, "Error: shmctl(shmPidID)\n");
        return -1;
    }

    return retval;
}


/************************* socket *************************/

int sockfd;
int connfd;

int passivesock(const char *service, const char *transport, int qlen)
{
    struct servent *pse;        /* pointer to service information entry */
    struct sockaddr_in sin;     /* an Internet endpoint address */
    int s, type;                /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port));
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("Can't find \"%s\" service entry\n", service);
    
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0) 
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("Can't create socket: %s\n", strerror(errno));
    
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't bind to port %s: %s\n", service, strerror(errno));

    /* Set the maximum number of waiting connection */
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("Can't listen on port %s: %s\n", service, strerror(errno));
    
    return s;
}

int connectsock(const char *host, const char *service, const char *transport)
{
    struct hostent *phe;        /* pointer to host information entry */
    struct servent *pse;        /* pointer to service information entry */
    struct sockaddr_in sin;     /* an Internet endpoint address */
    int s, type;                /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* Map service name to port number */
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("Can't find \"%s\" service entry\n", service);
    
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0) 
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("Can't create socket: %s\n", strerror(errno));
    
    /* Connect the socket */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't connect to %s:%s : %s\n", host, service, strerror(errno));

    return s;
}



/************************* signal *************************/

void SIGINT_handler(int signum) {
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Destroy share memory segments */
    int retval;
    retval = sharedMemoryRemove();
    if (retval < 0)
        fprintf(stderr, "[Interrupt Handler] [WARN_INFO]\n  Warning: Failed to remove all shared memory segments.\n\n");    
    
    /* Remove semaphores */
    retval = semaphoreRemove();
    if (retval < 0)
        fprintf(stderr, "[Interrupt Handler] [WARN_INFO]\n  Warning: Failed to remove all semaphores.\n\n");

}

void SIGCHLD_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

