#include "process_lib.h"

/*************************************************************************
 *                      client process function set                      *
**************************************************************************/

void clientProcess(const int connfd) {
    /* CDC system variables */
    int mild_case, severe_case, number_case, region_number;
    unsigned short is_severe = 0, is_new_case = 0;
    double waiting_time, largest_waiting_time;
    int case_number_array[2][NUM_REGION];
    Node temp_node;

    /* process variables */
    pid_t myPID = getpid();

    /* socket variables */
    char snd[BUF_SIZE], rcv[BUF_SIZE];
    char command[BUF_SIZE][STRING_SIZE], temp[STRING_SIZE];
    int command_length = 0;

    /* other trivial variables */
    int n, i;

    while (1) {
        /* Read user commands from the client */
        memset(rcv, 0, BUF_SIZE);
        if ((n = read(connfd, rcv, BUF_SIZE)) == -1)
            goto F_ERR_READ;
        rcv[n] = '\0';

        if (DEBUG)
            printf("[process: %d] Server receives: %s\n", myPID, rcv);
        
        /* Split the received string w.r.t. '|' */
        command_length = 0;
        char *p = strtok(rcv, " |\n");
        if (DEBUG)  printf("[process: %d] Parsing the string...\n", myPID);
        while (p != NULL) {
            if (DEBUG)  printf("[process: %d]\t token: %s\n", myPID, p);
            strcpy(command[command_length++], p);
            p = strtok(NULL, " |\n");
        }
        if (DEBUG)  printf("[process: %d]\t command length = %d\n", myPID, command_length);

        
        /* 0. List the menu */
        memset(snd, 0, BUF_SIZE);
        if (strcmp(command[0], "list") == 0) {
            n = sprintf(snd, "%s", "1. Confirmed case\n2. Reporting system\n3. Exit\n");

            if ((n = write(connfd, snd, strlen(snd)+1)) == -1)
                goto F_ERR_WRITE;
        }
        /* 1. Confirmed case */
        else if (strcmp(command[0], "Confirmed") == 0) {
            
            /* 1-1. List the number of confirmed cases of all regions */
            if (command_length == 2) {
                for (i = 0; i < NUM_REGION; i++) {
                    severe_case = getConfirmedData(i, 1);
                    mild_case = getConfirmedData(i, 0);
                    n = sprintf(temp, "%d : %d\n", i, severe_case + mild_case);
                    strcat(snd, temp);
                }

                if ((n = write(connfd, snd, strlen(snd)+1)) == -1)
                    goto F_ERR_WRITE;
                
            }
            /* 1-2. List the number of confirmed cases of specific regions */
            else if (command_length % 2 == 0) {
                for (i = 0; i < command_length / 2 - 1; i++) {
                    if (strstr(command[(i+1)*2], "Area") == NULL)
                        goto F_ERR_INPUT_CONFIRMED_CASE;

                    // Obtain region number
                    region_number = command[(i+1)*2+1][0] - '0';
                    if (region_number < 0 || region_number >= NUM_REGION)
                        goto F_ERR_OUTOFBOUND;

                    // Obtain known confirmed cases of the specific region
                    severe_case = getConfirmedData(region_number, 1);
                    mild_case = getConfirmedData(region_number, 0);
                    
                    n = sprintf(temp, "Area %d - Severe : %d | Mild : %d\n", region_number, severe_case, mild_case);
                    strcat(snd, temp);
                }

                if ((n = write(connfd, snd, strlen(snd)+1)) == -1)
                    goto F_ERR_WRITE;

                if (DEBUG)  printf("[process: %d]\t Server sends: %s\n", myPID, snd);
            } 
            /* 1-3. User command format error -> exit */
            else {
                goto F_ERR_INPUT_CONFIRMED_CASE;
            }
        /* 2. Reporting system */
        } else if (strcmp(command[0], "Reporting") == 0) {

            /* 2-1. Report the case number of specific degree (Mild/Severe) in specific region */
            if (command_length % 2 == 0 && command_length > 2) {
                
                i = 2;
                while (i < command_length) {
                    if (strstr(command[i++], "Area") == NULL) {
                        goto F_ERR_INPUT_REPORTING_SYSTEM;
                    } else {
                        // Obtain the region of reported confirmed cases
                        region_number = command[i++][0] - '0';
                        if (region_number < 0 || region_number >= NUM_REGION)
                            goto F_ERR_OUTOFBOUND;
                        
                        if (strstr(command[i], "Mild") == NULL && strstr(command[i], "Severe") == NULL)
                            goto F_ERR_INPUT_REPORTING_SYSTEM;
                        
                        // Add new cases to the buffer
                        is_severe = (strstr(command[i++], "Severe") != NULL);
                        number_case = atoi(command[i++]);

                        case_number_array[1 - is_severe][region_number] += number_case;
                        addConfirmedData(region_number, is_severe, number_case);

                        if (i >= command_length)
                            break;

                        // Check if another type of confirmed cases is reported
                        if (strstr(command[i], "Mild") != NULL || strstr(command[i], "Severe") != NULL) {
                            // Add new cases to the buffer
                            is_severe = (strstr(command[i++], "Severe") != NULL);
                            number_case = atoi(command[i++]);

                            case_number_array[1 - is_severe][region_number] += number_case;
                            addConfirmedData(region_number, is_severe, number_case);
                        }
                    }
                }

                largest_waiting_time = -1;
                for (i = 0; i < NUM_REGION; i++) {
                    if (case_number_array[0][i] != 0 && case_number_array[1][i] != 0) {
                        n = sprintf(temp, "Area %d | Severe %d | Mild %d\n", 
                            i, case_number_array[0][i], case_number_array[1][i]);
                        is_new_case = 1;
                    } else if (case_number_array[0][i] != 0) {
                        n = sprintf(temp, "Area %d | Severe %d\n", i, case_number_array[0][i]);
                        is_new_case = 1;
                    } else if (case_number_array[1][i] != 0) {
                        n = sprintf(temp, "Area %d | Mild %d\n", i, case_number_array[1][i]);
                        is_new_case = 1;
                    } else {
                        is_new_case = 0;
                    }

                    if (is_new_case) {
                        strcat(snd, temp);
                        temp_node = initNode(i, case_number_array[0][i], case_number_array[1][i], 0);

                        waiting_time = getAmbulanceWaitingTime(temp_node);
                        if (waiting_time < 0)
                            goto F_ERR_TIME;
                        
                        largest_waiting_time = (largest_waiting_time < waiting_time) ? waiting_time : largest_waiting_time;
                    }
                }
                // waiting messages
                strcpy(temp, "The ambulance is on it's way...\n");
                if (write(connfd, temp, strlen(temp)+1) == -1) {
                    if (DEBUG)  fprintf(stderr, "[Reporting system (waiting message)] ");
                    goto F_ERR_WRITE;
                }
                if (DEBUG)  printf("[process: %d] Server sends: %s\n", myPID, temp);

                if (DEBUG)  printf("[process: %d] Sleep for %f seconds\n", myPID, largest_waiting_time);
                sleep((int) largest_waiting_time);   // sleep for <largest_waiting_time> seconds
                
                // result messages
                if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
                    if (DEBUG)  fprintf(stderr, "[Reporting system (result message)] ");
                    goto F_ERR_WRITE;
                }
            } else {
                // User command format error -> exit
                goto F_ERR_INPUT_REPORTING_SYSTEM;
            }

        }
        /* 3. Exit the system */
        else if (strcmp(command[0], "Exit") == 0) {
            goto F_EXIT;
        }
        /* other: invalid command */
        else {
            goto F_ERR_COMMAND;
        }

        if (DEBUG)
            printf("[process: %d] Server sends: %s\n", myPID, snd);

        // Clear the command line buffer
        for (i = 0; i < command_length; i++)
            memset(command[i], 0, STRING_SIZE);
        
        // Clear the case number array
        for (i = 0; i < NUM_REGION; i++) {
            case_number_array[0][i] = 0;
            case_number_array[1][i] = 0;
        }
    }
    
F_ERR_READ:
    fprintf(stderr, "[process: %d] Error: read()\n", myPID);
    goto F_EXIT;
F_ERR_WRITE:
    fprintf(stderr, "[process: %d] Error: write()\n", myPID);
    goto F_EXIT;
F_ERR_COMMAND:
    fprintf(stderr, "[process: %d] Error: invalid command\n", myPID);
    goto F_EXIT;
F_ERR_INPUT_CONFIRMED_CASE:
    fprintf(stderr, "[process: %d] Usage: Confirmed case [ | Area <region number> | Area <region number> | ...]\n", myPID);
    fprintf(stderr, "[process: %d]\t each option should be split by an \'|\'\n", myPID);
    goto F_EXIT;
F_ERR_INPUT_REPORTING_SYSTEM:
    fprintf(stderr, "[process: %d] Usage: Reporting system | Area <region number> | Mild/Severe <case number> | [...]\n", myPID);
    fprintf(stderr, "[process: %d]\t each option should be split by an \'|\'\n", myPID);
    goto F_EXIT;
F_ERR_OUTOFBOUND:
    fprintf(stderr, "[process: %d] Error: array out of bound\n", myPID);
    goto F_EXIT;
F_ERR_TIME:
    fprintf(stderr, "[process: %d] Error: cannot get waiting time\n", myPID);
    goto F_EXIT;

F_EXIT:
    printf("[process: %d] exiting...\n\n", myPID);
    close(connfd);
    exit(0);
}

int getConfirmedData(int region_num, int isSevere) {
    int ret = -1;

    if (sem_acquire(semaCDCSystem) < 0) {
        fprintf(stderr, "[getConfirmedData()]\t Error: sem_acquire()\n");
        goto F_EXIT;
    }
    
    int *shm;
    if (isSevere) {
        if ((shm = shmat(shmSevereCaseID, NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[getConfirmedData()]\t Error: shmat(shmSevereCaseID)\n");
            goto F_EXIT;
        }
    } else {
        if ((shm = shmat(shmMildCaseID, NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[getConfirmedData()]\t Error: shmat(shmMildCaseID)\n");
            goto F_EXIT;
        }
    }

    // Obtain current confirmed case number of the specific region
    ret = *(shm + region_num);

    shmdt(shm);

F_ERR_SHM:
    if (sem_release(semaCDCSystem) < 0) {
        fprintf(stderr, "[getConfirmedData()]\t Error: sem_release()\n");
        goto F_EXIT;
    }

F_EXIT:
    return ret;
}

void addConfirmedData(int region_num, int isSevere, int num_case) {

    if (sem_acquire(semaCDCSystem) < 0) {
        fprintf(stderr, "[addConfirmedData()]\t Error: sem_acquire()\n");
        goto F_EXIT;
    }

    int *shm;
    if (isSevere) {
        if ((shm = shmat(shmSevereCaseID, NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[addConfirmedData()]\t Error: shmat(shmSevereCaseID)\n");
            goto F_ERR_SHM;
        }
    } else {
        if ((shm = shmat(shmMildCaseID, NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[addConfirmedData()]\t Error: shmat(shmMildCaseID)\n");
            goto F_ERR_SHM;
        }
    }

    // Add new reported case number to the segment
    *(shm + region_num) += num_case;

    shmdt(shm);

F_ERR_SHM:
    if (sem_release(semaCDCSystem) < 0) {
        fprintf(stderr, "[addConfirmedData()]\t Error: sem_release()\n");
        goto F_EXIT;
    }

F_EXIT:
    return;
}


/************************* ambulance process function set *************************/

void ambulanceProcess(const int ID) {
    /* Time variables */
    unsigned short first_get_time = 1;
    struct timespec start, finish;

    /* semaphore variables */
    int sem_result;

    /* shared memory segments */
    List *shm_result;
    Node temp_node;

    /* process variables */
    pid_t myPID = getpid();

    printf("[ambulanceProcess: %d] Ambulance %d is ready!\n", myPID, ID);
    while (1) {
        /* Check if there is appending data in busX_buffer */

        // Use the specific semaphore
        // Attach shared memory segment according to ID
        if (ID == 1) {
            sem_result = semaBus1Result;
            if (sem_acquire(sem_result) < 0) {
                fprintf(stderr, "[ambulanceProcess: %d] Error: sem_acquire(semaBus1Result)\n", myPID);
                goto F_ERR_SEM;
            }
            if ((shm_result = shmat(shmBus1ResultID, NULL, 0)) == (void *) -1) {
                fprintf(stderr, "[ambulanceProcess: %d] Error: shmat(shmBus1ResultID)\n", myPID);
                goto F_ERR_SHM;
            }

        } else if (ID == 2){
            sem_result = semaBus2Result;
            if (sem_acquire(sem_result) < 0) {
                fprintf(stderr, "[ambulanceProcess: %d] Error: sem_acquire(semaBus2Result)\n", myPID);
                goto F_ERR_SEM;
            }
            if ((shm_result = shmat(shmBus2ResultID, NULL, 0)) == (void *) -1) {
                fprintf(stderr, "[ambulanceProcess: %d] Error: shmat(shmBus2ResultID)\n", myPID);
                goto F_ERR_SHM;
            }
        }

        /* Simulating the operation of ambulances */
        if (!isEmptyList(*shm_result)) {
            if (first_get_time) {
                clock_gettime(CLOCK_REALTIME, &start);
                first_get_time = 0;
            }

            clock_gettime(CLOCK_REALTIME, &finish);
            shm_result->nodes[0].execution_time = finish.tv_sec - start.tv_sec;
            shm_result->nodes[0].execution_time += (finish.tv_nsec - start.tv_nsec) / 1e09;

            // The ambulance completes its service
            if (shm_result->nodes[0].execution_time >= shm_result->nodes[0].region_number) {
                deleteBegin(shm_result);
                first_get_time = 1;
            }

        } else {
            first_get_time = 1;
        }
        
        shmdt(shm_result);
        
        sem_release(sem_result);

        // usleep(1000);   // sleep for 1 millisecond (to avoid frequently access to shared memory)
    }

F_EXIT:
    shmdt(shm_result);
F_ERR_SHM:
    sem_release(sem_result);
F_ERR_SEM:
    exit(0);
}


double getAmbulanceWaitingTime(Node newNode) {
    List *shm_bus1, *shm_bus2;

    // semaphore
    if (sem_acquire(semaBus1Result) < 0) {
        fprintf(stderr, "[getAmbulanceWaitingTime()] Error: sem_acquire(semaBus1Result)\n");
        goto F_ERR_SEM1;
    }
    if (sem_acquire(semaBus2Result) < 0) {
        fprintf(stderr, "[getAmbulanceWaitingTime()] Error: sem_acquire(semaBus2Result)\n");
        goto F_ERR_SEM2;
    }

    // shared memory segment
    if ((shm_bus1 = shmat(shmBus1ResultID, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "[getAmbulanceWaitingTime()] Error: shmat(shmBus1ResultID)\n");
        goto F_ERR_SHM1;
    }
    if ((shm_bus2 = shmat(shmBus2ResultID, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "[getAmbulanceWaitingTime()] Error: shmat(shmBus2ResultID)\n");
        goto F_ERR_SHM2;
    }

    double waiting_time1 = getNodeWaitingTime(*shm_bus1, newNode);
    double waiting_time2 = getNodeWaitingTime(*shm_bus2, newNode);

    // Add new reported case to the list that need less waiting time
    if (waiting_time1 <= waiting_time2) {
        addNewCaseNumber(shm_bus1, newNode);
    } else {
        addNewCaseNumber(shm_bus2, newNode);
    }

    if (DEBUG) {
        printf("[getAmbulanceWaitingTime()]\n\n");
        printf("Task waiting list of ambulance 1:\n\n");
        printList(*shm_bus1);

        printf("Task waiting list of ambulance 2:\n\n");
        printList(*shm_bus2);
    }


    shmdt(shm_bus1);
    shmdt(shm_bus2);
        
    sem_release(semaBus1Result);
    sem_release(semaBus2Result);

    return (waiting_time1 <= waiting_time2) ? waiting_time1 : waiting_time2;

F_ERR_SHM2:
    shmdt(shm_bus1);
F_ERR_SHM1:
    sem_release(semaBus2Result);
F_ERR_SEM2:
    sem_release(semaBus1Result);
F_ERR_SEM1:
    return -1;
}


/************************* signal registering function set *************************/

void interrupt_handler(int signum) {
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Destroy share memory segments */
    int i, retval;
    retval = sharedMemoryRemove();
    if (retval < 0)
        fprintf(stderr, "Error: Failed to remove all shared memory\n");    
    
    /* Remove semaphores */
    retval = semaphoreRemove();
    if (retval < 0)
        fprintf(stderr, "Error: Failed to remove all semaphores\n");
    
}

void zombie_process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
