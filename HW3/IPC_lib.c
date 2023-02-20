#include "IPC_lib.h"

/************************* semaphore *************************/

int semaphoreInit() {
    int semaphore_system_key = SEM_SYS_KEY;
    int semaphore_buffer_key = SEM_BUF_KEY;

    // semaphore for CDC system
    semaCDCSystem = semget(semaphore_system_key, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaCDCSystem < 0) {
        fprintf(stderr, "Error: unable to create semaphore %d\n", semaphore_system_key);
        return -1;
    }
    if (semctl(semaCDCSystem, 0, SETVAL, 1) < 0) {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphore_system_key);
        return -1;
    }

    // semaphore for bus 1 scheduling results
    semaBus1Result = semget(semaphore_buffer_key, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaBus1Result < 0) {
        fprintf(stderr, "Error: unable to create semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    if (semctl(semaBus1Result, 0, SETVAL, 1) < 0) {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    semaphore_buffer_key++;

    // semaphore for bus 2 scheduling results
    semaBus2Result = semget(semaphore_buffer_key, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (semaBus2Result < 0) {
        fprintf(stderr, "Error: unable to create semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    if (semctl(semaBus2Result, 0, SETVAL, 1) < 0) {
        fprintf(stderr, "Error: unable to set initial value 1 to semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    semaphore_buffer_key++;

    return 0;
}

int semaphoreRemove() {
    int semaphore_system_key = SEM_SYS_KEY;
    int semaphore_buffer_key = SEM_BUF_KEY;

    if (semctl (semaCDCSystem, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "Error: unable to remove semaphore %d\n", semaphore_system_key);
        return -1;
    }

    if (semctl (semaBus1Result, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "Error: unable to remove semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    semaphore_buffer_key++;

    if (semctl (semaBus2Result, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "Error: unable to remove semaphore %d\n", semaphore_buffer_key);
        return -1;
    }
    semaphore_buffer_key++;

    return 0;
}


int sem_acquire(int sem) {
    struct sembuf sop;  /* the operation parameters */
    sop.sem_num = 0;    /* access the 1st (and only) sem in the array */
    sop.sem_op = -1;    /* wait..*/
    sop.sem_flg = 0;    /* no special options needed */

    if (semop (sem, &sop, 1) < 0) {
        fprintf(stderr, "Error: sem_acquire() failed\n");
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
        fprintf(stderr, "Error: sem_release() failed\n");
        return -1;
    }
    
    return 0;
}


/************************* shared memory *************************/


int sharedMemoryInit() {
    int shm_system_key = SHM_SYS_KEY;
    int shm_buffer_key = SHM_BUF_KEY;

    // shmSevereCase key: 1000
    if ((shmSevereCaseID = shmget(shm_system_key++, sizeof(int) * NUM_REGION, IPC_CREAT | SEM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget(shmSevereCaseID)\n");
        return -1;
    }

    // shmMildCase key: 1001
    if ((shmMildCaseID = shmget(shm_system_key++, sizeof(int) * NUM_REGION, IPC_CREAT | SEM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget(shmMildCaseID)\n");
        return -1;
    }
    
    // shmBus1ResultID key: 3000
    if ((shmBus1ResultID = shmget(shm_buffer_key++, sizeof(List), IPC_CREAT | SEM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget(shmBus1ResultID)\n");
        return -1;
    }

    // shmBus2ResultID key: 3001
    if ((shmBus2ResultID = shmget(shm_buffer_key++, sizeof(List), IPC_CREAT | SEM_MODE)) < 0) {
        fprintf(stderr, "Error: shmget(shmBus2ResultID)\n");
        return -1;
    }
    
    return 0;
}


int sharedMemoryRemove() {
    if (shmctl(shmSevereCaseID, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: shmctl(shmSevereCaseID)\n");
        return -1;
    }
        
    if (shmctl(shmMildCaseID, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: shmctl(shmMildCaseID)\n");
        return -1;
    }

    if (shmctl(shmBus1ResultID, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: shmctl(shmBus1ResultID)\n");
        return -1;
    }
    
    if (shmctl(shmBus2ResultID, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Error: shmctl(shmBus2ResultID)\n");
        return -1;
    }

    return 0;
}