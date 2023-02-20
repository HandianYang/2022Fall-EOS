#include "ElevatorOperation.h"

/*************************** Process function set ***************************/

void elevatorSimulationProcess(const unsigned long elevatorID) {
    /* elevator simulation */
    Call target_call, temp_call, head_call, accepted_lift_call;
    CallList deletedFloorCallList;
    Elevator *elevator;
    Floor *floor;
    unsigned int floor_call_count, lift_call_count;
    int free_space;

    /* time */
    unsigned short is_begin_of_transition = 0;
    double transition_time, elapsed_time;
    struct timespec start, finish;

    /* others */
    int i;

    /* Process loop */
    while (1) {
        /* Acquire the semaphore */
        if (sem_acquire(semaphoreElevator[elevatorID]) < 0) {
            fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: sem_acquire()\n\n", elevatorID);
            goto F_ERR_SEM_E;
        }
        /* Attach the shared memory segment */
        if ((elevator = shmat(shmElevatorID[elevatorID], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: shmat()\n\n", elevatorID);
            goto F_ERR_SHM_E;
        }

        if (elevator->abnormal == 1) {
            sem_release(semaphoreElevator[elevatorID]);
            shmdt(elevator);
            continue;
        }

        transition_time = FLOOR_HEIGHT / elevator->velocity;

        /* Begin operation if there exists some call(s) */
        if (!isEmptyCallList(elevator->callSequence)) {

            /* Obtain the first call from callSequence (non-destructive)
             * 
             * [Note] the first call can be preempted by any another calls with higher priority
             *      at any transition time
             */
            target_call = getIndexLinkedCall(elevator->callSequence, 0);

            elevator->idle = 0;

            if (elevator->direction == 0)
                elevator->direction = target_call.direction;

            /* STOP */
            if (target_call.floor == elevator->current_floor) {
                printf("[Elevator %ld] [DEBUG_INFO]\n  Elevator stops at %d.\n\n", elevatorID, target_call.floor);

                /* Open the door */
                elevatorOperationSleep(elevatorID, 1);  // door-open sleep

                /* Proceed all calls-to-be-handled */
                lift_call_count = 0;
                floor_call_count = 0;

                for (i = 0; i < elevator->callSequence.length; i++) {
                    
                    temp_call = getIndexLinkedCall(elevator->callSequence, i);

                    if (temp_call.direction == target_call.direction && temp_call.floor == target_call.floor) {
                        if (temp_call.isFloorCall)
                            floor_call_count++;
                        else
                            lift_call_count++;
                    }
                }

                free_space = elevator->capacity - elevator->load - lift_call_count;
                floor_call_count = (floor_call_count > free_space) ? free_space : floor_call_count;

                printf("[Elevator %ld] [DEBUG_INFO]\n", elevatorID);
                printf("  The number of floor calls: %d\n", floor_call_count);
                printf("  The number of lift calls: %d\n", lift_call_count);

                /* Release passengers: handle deleted lift calls */
                while (lift_call_count--) {
                    head_call = deleteBeginLinkedCall(&(elevator->callSequence));

                    if (!head_call.isSpecialCall) {
                        if (elevator->load > 0)
                            elevator->load--;
                        
                        printf("[Elevator %ld] [DEBUG_INFO]\n  Passenger (ID=%ld) has been released.\n\n", elevatorID, head_call.ID);

                        // unloading sleep
                        elevatorOperationSleep(elevatorID, elevator->passenger_unloading_time);
                    }
                }

                /* Accept passengers: handle deleted floor calls */
                deletedFloorCallList.length = 0;
                while (floor_call_count--) {
                    head_call = deleteBeginLinkedCall(&(elevator->callSequence));
                    insertEndLinkedCall(&deletedFloorCallList, head_call);
                }

                for (i = 0; i < deletedFloorCallList.length; i++) {
                    /********************** criteria **********************/
                    if (sem_acquire(semaphoreFloor[elevator->current_floor - 1]) < 0) {
                        fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: sem_acquire()\n\n", elevatorID);
                        goto F_ERR_SEM_F;
                    }

                    if ((floor = shmat(shmFloorID[elevator->current_floor - 1], NULL, 0)) == (void *) -1) {
                        fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: shmat()\n\n", elevatorID);
                        goto F_ERR_SHM_F;
                    }

                    // Delete the specific passenger at the current floor from passenger list
                    deleteIDLinkedPassenger(&(floor->passengers), deletedFloorCallList.nodes[i].ID);

                    shmdt(floor);
                    sem_release(semaphoreFloor[elevator->current_floor - 1]);
                    /*****************************************************/

                    // load
                    if (elevator->load < elevator->capacity)
                        elevator->load++;
                    // if (elevator->load > elevator->capacity) {
                    //     fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: load should not be larger than %ld.\n\n", elevatorID, elevator->capacity);
                    //     goto F_ERR_EXIT;
                    // }

                    /* Transfer lift calls from liftCall to callSequence */
                    accepted_lift_call = deleteIDLinkedCall(&(elevator->liftCalls), deletedFloorCallList.nodes[i].ID);

                    accepted_lift_call.passage = 1;
                    insertPriorityCall(&(elevator->callSequence), accepted_lift_call);

                    printf("[Elevator %ld] [DEBUG_INFO]\n  Passenger (ID=%ld) has been accepted.\n\n", elevatorID, accepted_lift_call.ID);

                    // loading sleep
                    elevatorOperationSleep(elevatorID, elevator->passenger_loading_time);
                }
                
                /* Close the door */
                elevatorOperationSleep(elevatorID, 1);  // door-closed sleep
            }
            /* TRANSITION */
            else {
                /* Change the direction of the elevator */
                // System finds the next call is invoked at the floor lower than current floor
                if (target_call.floor < elevator->current_floor && elevator->direction != -1) {
                    elevator->direction = -1;           // changes direction to 'down'
                    reassignPassagePriority(elevator);  // re-assign passage priority (because of direction change)
                }
                // System finds the next call is invoked at the floor higher than current floor
                else if (target_call.floor > elevator->current_floor && elevator->direction != 1) {
                    elevator->direction = 1;            // changes direction to 'up'
                    reassignPassagePriority(elevator);  // re-assign passage priority (because of direction change)
                }

                /* Simulate elevator movement through the floors of the building */
                if (elevator->current_floor != target_call.floor
                    && elevator->current_floor >= 1
                    && elevator->current_floor <= NUM_OF_FLOOR) {

                    /* Calculate the elapsed time for transition */
                    if (is_begin_of_transition) {
                        clock_gettime(CLOCK_REALTIME, &start);
                        is_begin_of_transition = 0;
                    }

                    clock_gettime(CLOCK_REALTIME, &finish);
                    elapsed_time = finish.tv_sec - start.tv_sec;
                    elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1e09;

                    /* One transition between floors is completed */
                    if (elapsed_time >= transition_time) {
                        // up direction
                        if (elevator->direction == 1 && elevator->current_floor < NUM_OF_FLOOR) {
                            elevator->current_floor += 1;
                        }
                        // down direction
                        else if (elevator->direction == -1 && elevator->current_floor > 1) {
                            elevator->current_floor -= 1;
                        }
                        else {
                            fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n", elevatorID);
                            fprintf(stderr, "  Error: floor number %d is out of bound!\n\n", target_call.floor);
                            // printf("[Elevator %ld] [DEBUG_INFO]\n  Elevator direction: %s.\n\n", elevatorID, elevator->direction == 1 ? "up" : "down");
                            goto F_ERR_EXIT;
                        }

                        is_begin_of_transition = 1;
                        printf("[Elevator %ld] [DEBUG_INFO]\n  Elevator transits to %d floor.\n\n", elevatorID, elevator->current_floor);
                    }
                }
            }
        } else {
            elevator->idle = 1;
            elevator->direction = 0;
        }

        /* Detach the shared memory segment */
        shmdt(elevator);

        /* Release the semaphore */
        sem_release(semaphoreElevator[elevatorID]);
        
    }
    exit(0);

F_ERR_EXIT:
    shmdt(floor);
F_ERR_SHM_F:
    sem_release(semaphoreFloor[elevator->current_floor - 1]);
F_ERR_SEM_F:
    shmdt(elevator);
F_ERR_SHM_E:
    sem_release(semaphoreElevator[elevatorID]);
F_ERR_SEM_E:
    exit(-1);
}


void elevatorAssignFloorCallProcess(const unsigned long elevatorID) {
    /* elevator calls sorting */
    Call target_floor_call;
    Elevator *elevator;

    while (1) {
        /* Acquire the semaphore */
        if (sem_acquire(semaphoreElevator[elevatorID]) < 0) {
            fprintf(stderr, "[Elevator %ld] [elevatorAssignFloorCallProcess] [ERR_INFO]\n", elevatorID);
            fprintf(stderr, "  Error: sem_acquire()\n\n");
            goto F_ERR_SEM;
        }
        /* Attach the shared memory segment */
        if ((elevator = shmat(shmElevatorID[elevatorID], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Elevator %ld] [elevatorAssignFloorCallProcess] [ERR_INFO]\n", elevatorID);
            fprintf(stderr, "  Error: shmat()\n\n");
            goto F_ERR_SHM;
        }

        if (!isEmptyCallList(elevator->floorCalls)) {
            
            target_floor_call = deleteBeginLinkedCall(&(elevator->floorCalls));

            printf("[Elevator %ld] [elevatorAssignFloorCallProcess] [DEBUG_INFO]\n", elevatorID);
            printf("  New floor call found at floor %d\n", target_floor_call.floor);

            /* Assign passage to a newly arrived floorCall */
            if (elevator->direction == 1) {
                
                if ((target_floor_call.floor > elevator->current_floor) && (target_floor_call.direction == elevator->direction)) {
                    // P1 : the call is the same direction (up) as the elevator
                    //      and is higher than the elevator's current floor
                    target_floor_call.passage = 1;
                }
                else if ((target_floor_call.floor < elevator->current_floor) && (target_floor_call.direction == elevator->direction)) {
                    // P3 : the call is the same direction (up) as the elevator
                    //      and is lower than the elevator's current floor
                    target_floor_call.passage = 3;
                }
                else {
                    // P2 : the call has opposite direction to the elevator
                    target_floor_call.passage = 2;
                }
            }
            else if (elevator->direction == -1) {
                if ((target_floor_call.floor < elevator->current_floor) && (target_floor_call.direction == elevator->direction)) {
                    // P1 : the call is the same direction (down) as the elevator
                    //      and is lower than the elevator's current floor
                    target_floor_call.passage = 1;
                }
                else if ((target_floor_call.floor > elevator->current_floor) && (target_floor_call.direction == elevator->direction)) {
                    // P3 : the call is the same direction (up) as the elevator
                    //      and is higher than the elevator's current floor
                    target_floor_call.passage = 3;
                }                
                else {
                    // P2 : the call has opposite direction to the elevator
                    target_floor_call.passage = 2;
                }  
            }

            insertPriorityCall(&(elevator->callSequence), target_floor_call);
        }

        /* Detach the shared memory segment */
        shmdt(elevator);

        /* Release the semaphore */
        sem_release(semaphoreElevator[elevatorID]);

        usleep(0.05 * 1e06);
    }

F_ERR_EXIT:
    shmdt(elevator);
F_ERR_SHM:
    sem_release(semaphoreElevator[elevatorID]);
F_ERR_SEM:
    exit(0);
}


void elevatorSchedulerProcess(const int algorithm) {
    int i, selected_elevator;
    unsigned int start_index = 0;

    Elevator *elevatorPointerGroup[NUM_OF_ELEVATOR];
    Elevator elevatorGroup[NUM_OF_ELEVATOR];
    Floor *floor;
    Passenger new_passenger;
    
    while (1) {
        /* Only acquire the semaphore of <start_index> floor */
        if (sem_acquire(semaphoreFloor[start_index]) < 0) {
            fprintf(stderr, "[ElevatorGroup] [elevatorSchedulerProcess] [ERR_INFO]\n");
            fprintf(stderr, "  Error: sem_acquire(floor)\n\n");
            goto F_ERR_SEM_F;
        }

        /* Only attach the shared memory segment of <start_index> floor */
        if ((floor = shmat(shmFloorID[start_index], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[ElevatorGroup] [elevatorSchedulerProcess] [ERR_INFO]\n");
            fprintf(stderr, "  Error: shmat(floor)\n\n");
            goto F_ERR_SHM_F;
        }

        /* Do scheduling if an unchecked passenger is found */
        if (!isEmptyPassengerList(floor->passengers_unchecked)) {

            /* Transfer new passenger from unchecked list to checked list */
            new_passenger = deleteBeginLinkedPassenger(&(floor->passengers_unchecked));
            insertEndLinkedPassenger(&(floor->passengers), new_passenger);

            /* Acquire the semaphores of all elevators */
            for (i = 0; i < NUM_OF_ELEVATOR; i++) {
                if (sem_acquire(semaphoreElevator[i]) < 0) {
                    fprintf(stderr, "[ElevatorGroup] [elevatorSchedulerProcess] [ERR_INFO]\n");
                    fprintf(stderr, "  Error: sem_acquire(elevator)\n\n");
                    goto F_ERR_SEM_E;
                }
            }

            /* Attach the shared memory segment */
            for (i = 0; i < NUM_OF_ELEVATOR; i++) {
                if ((elevatorPointerGroup[i] = shmat(shmElevatorID[i], NULL, 0)) == (void *) -1) {
                    fprintf(stderr, "[ElevatorGroup] [elevatorSchedulerProcess] [ERR_INFO]\n");
                    fprintf(stderr, "  Error: shmat(elevator)\n");
                    goto F_ERR_SHM_E;
                }

                elevatorGroup[i] = *(elevatorPointerGroup[i]);
            }

            /* Do scheduling */
            switch (algorithm) {
            case 1:     // 1 - Round-Robin policy
                selected_elevator = policyRoundRobin(elevatorGroup);
                break;
            case 2:     // 2 - Up-peak policy (variation of Round-Robin) 
                selected_elevator = policyRoundRobin(elevatorGroup);
                break;
            case 3:     // 3 - Three-Passage policy
                selected_elevator = policyThreePassage(elevatorGroup);
                break;
            case 4:     // 4 - Zoning policy
                selected_elevator = policyZoning(new_passenger.floor_call.floor);
                break;
            default:    // Set Round-Robin policy as default
                selected_elevator = policyRoundRobin(elevatorGroup);
                break;
            }

            /* Append to the elevator's calls */
            elevatorAppendPassengerCall(elevatorPointerGroup[selected_elevator], new_passenger);

            /* Detach the shared memory segments */
            for (i = 0; i < NUM_OF_ELEVATOR; i++)
                shmdt(elevatorPointerGroup[i]);

            /* Release the semaphores */
            for (i = 0; i < NUM_OF_ELEVATOR; i++)
                sem_release(semaphoreElevator[i]);
        } else {
            start_index = (start_index + 1) % NUM_OF_FLOOR;
        }
        
        shmdt(floor);

        sem_release(semaphoreFloor[start_index]);

        usleep(0.05 * 1e06);     // check passengers for every 0.05 seconds
    }

F_ERR_EXIT:
    for (i = 0; i < NUM_OF_ELEVATOR; i++)
        shmdt(elevatorPointerGroup[i]);
F_ERR_SHM_E:
    for (i = 0; i < NUM_OF_ELEVATOR; i++)
        sem_release(semaphoreElevator[i]);
F_ERR_SEM_E:
    shmdt(floor);
F_ERR_SHM_F:
    sem_release(semaphoreFloor[start_index]);
F_ERR_SEM_F:
    exit(-1);
}


void elevatorUpPeakProcess(const unsigned long elevatorID) {
   /* elevator */
    Elevator *elevator;

    /* time */
    unsigned short is_begin_of_timer = 1, up_peak_is_invoked = 0;
    double transition_time, elapsed_time;
    struct timespec start, finish;

    while (1) {
        /* Attach the shared memory segment */
        if ((elevator = shmat(shmElevatorID[elevatorID], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Elevator %ld] [elevatorUpPeakProcess]\n", elevatorID);
            fprintf(stderr, "  Error: sem_acquire()\n\n");
            goto F_ERR_SHM;
        }

        /* Drive the elevator back to the ground floor if the elevator keeps idle for 7 seconds */
        if (elevator->idle && elevator->current_floor > 1) {
            // start the timer
            if (is_begin_of_timer) {
                clock_gettime(CLOCK_REALTIME, &start);
                is_begin_of_timer = 0;
            }

            clock_gettime(CLOCK_REALTIME, &finish);
            elapsed_time = finish.tv_sec - start.tv_sec;
            elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1e09;
            
            // invoke the up-peak call if the elevator waits more than 7 seconds
            if (elapsed_time >= 7 && elevator->current_floor > 1) {
                
                Call temp_call = initCall(0, 0, 1, 1, 1, -1);
                
                /********************* elevator criteria  *********************/
                if (sem_acquire(semaphoreElevator[elevatorID]) < 0) {
                    fprintf(stderr, "[Elevator %ld] [elevatorUpPeakProcess]\n", elevatorID);
                    fprintf(stderr, "  Error: sem_acquire()\n\n");
                    goto F_ERR_SEM;
                }

                insertPriorityCall(&(elevator->callSequence), temp_call);

                sem_release(semaphoreElevator[elevatorID]);
                /********************* elevator criteria  *********************/

                up_peak_is_invoked = 1;
            }
        } else {
            is_begin_of_timer = 1;
        }

        /* Detach the shared memory segment */
        shmdt(elevator);

        /* Wait 3 seconds to make sure the up-peak call is scheduled and executed */
        if (up_peak_is_invoked) {
            sleep(3);
            up_peak_is_invoked = 0;
        }
    }

F_ERR_EXIT:
    shmdt(elevator);
F_ERR_SHM:
    sem_release(semaphoreElevator[elevatorID]);
F_ERR_SEM:
    exit(0);
}


void elevatorZoningProcess(const unsigned long elevatorID) {
   /* elevator */
    Elevator *elevator;
    int zone = (int) ceil((double) NUM_OF_FLOOR / NUM_OF_ELEVATOR);
    int zone_origin_floor = elevatorID * zone + 1;
    int direction;

    /* time */
    unsigned short is_begin_of_timer = 1, zoning_is_invoked = 0;
    double transition_time, elapsed_time;
    struct timespec start, finish;

    while (1) {
        /* Attach the shared memory segment */
        if ((elevator = shmat(shmElevatorID[elevatorID], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Elevator %ld] [elevatorZoningProcess]\n", elevatorID);
            fprintf(stderr, "  Error: shmat()\n\n");
            goto F_ERR_SHM;
        }

        /* Drive the elevator to the zone origin floor if the elevator keeps idle for 7 seconds */
        if (elevator->idle && elevator->current_floor != zone_origin_floor) {
            // start the timer
            if (is_begin_of_timer) {
                clock_gettime(CLOCK_REALTIME, &start);
                is_begin_of_timer = 0;
            }

            clock_gettime(CLOCK_REALTIME, &finish);
            elapsed_time = finish.tv_sec - start.tv_sec;
            elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1e09;
            
            // invoke the zoning call if the elevator waits more than 7 seconds
            if (elapsed_time >= 7 && elevator->current_floor != zone_origin_floor) {
                direction = (zone_origin_floor - elevator->current_floor) / abs(zone_origin_floor - elevator->current_floor);
                Call temp_call = initCall(0, 0, 1, 1, elevatorID * zone + 1, direction);

                /********************* elevator criteria  *********************/
                if (sem_acquire(semaphoreElevator[elevatorID]) < 0) {
                    fprintf(stderr, "[Elevator %ld] [elevatorZoningProcess]\n", elevatorID);
                    fprintf(stderr, "  Error: sem_acquire()\n\n");
                    goto F_ERR_SEM;
                }

                insertPriorityCall(&(elevator->callSequence), temp_call);

                sem_release(semaphoreElevator[elevatorID]);
                /********************* elevator criteria  *********************/

                zoning_is_invoked = 1;
            }
        } else {
            is_begin_of_timer = 1;
        }

        /* Detach the shared memory segment */
        shmdt(elevator);

        /* Wait 3 seconds to make sure the zoning call is scheduled and executed */
        if (zoning_is_invoked) {
            sleep(3);
            zoning_is_invoked = 0;
        }
    }

F_ERR_EXIT:
    shmdt(elevator);
F_ERR_SHM:
    sem_release(semaphoreElevator[elevatorID]);
F_ERR_SEM:
    exit(0);
}

struct timeval tv1;
struct timeval tv2;

void parentProcess(int sockfd,struct sockaddr_in addr_cln,socklen_t sLen)
{
    pid_t client_pid;
    gettimeofday(&tv1, NULL);
    while (1) {
        // Accept the connection of some client 
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1) {
            fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: accept(sockfd).\n\n");
            goto F_EXIT;
        }
        printf("[Main Server] [INFO]\n  Connection has been established!!\n\n");
        
        if((client_pid = fork()) == 0) {
            // child
            clientProcess(connfd);         
        } else if (client_pid < 0) {
            // error
            fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: fork()\n\n");
            goto F_EXIT;
        }
        
    }
F_EXIT:
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);
}

void clientProcess(const int connfd) {
    // trivial
    int i, j;
    double time = 0;
    double diff_time=0;

    // socket buffer
    char snd[BUF_SIZE], temp[BUF_SIZE];

    // elevator
    unsigned int elevator_load;
    Elevator *elevatorPointerGroup[NUM_OF_ELEVATOR];

    // floor
    unsigned int print_length, passengerList_length;
    Floor *floor;

    // process
    pid_t pid = getpid();

    while (1) {
        /* Attach the shared memory segments of all elevators */
        for (i = 0; i < NUM_OF_ELEVATOR; i++) {
            if ((elevatorPointerGroup[i] = shmat(shmElevatorID[i], NULL, 0)) == (void *) -1) {
                fprintf(stderr, "[Client %d] [ERR_INFO]\n", pid);
                fprintf(stderr, "  Error: shmat(elevator[%d])\n\n", i);
                goto F_EXIT;
            }
        }

        /* Display */
        gettimeofday(&tv2, NULL);
        diff_time=(tv2.tv_sec * 1000 + tv2.tv_usec / 1000)-(tv1.tv_sec * 1000 + tv1.tv_usec / 1000);
        diff_time/=1000;
        memset(snd, 0, BUF_SIZE);
        memset(temp, 0, BUF_SIZE);
        sprintf(snd, "[Client %d] Time : %.2f\n", pid, diff_time);

        for (i = NUM_OF_FLOOR; i > 0; i--) {
            // print floor's head
            strcat(snd, FLOOR_HEAD);
            
            sprintf(temp, "Floor %d\n", i);
            strcat(snd, temp);

            // print elevator's body
            strcat(snd, ELEVATOR_EMPTY_BODY);   strcat(snd, "\n");
            strcat(snd, ELEVATOR_EMPTY_BODY);   strcat(snd, "\n");

            sprintf(temp, ELEVATOR_EMPTY_BODY);
            for (j = 0; j < NUM_OF_ELEVATOR; j++) {
                if (elevatorPointerGroup[j]->current_floor == i) {
                    elevator_load = elevatorPointerGroup[j]->load;
                    temp[ELEVATOR_WIDTH * j + 4] = (elevator_load >= 10) ? (elevator_load / 10) % 10 + '0' : ' ';
                    temp[ELEVATOR_WIDTH * j + 5] = elevator_load % 10 + '0';
                    if (elevatorPointerGroup[j]->direction == 1)
                        temp[ELEVATOR_WIDTH * j + 7] = '^';
                    else if (elevatorPointerGroup[j]->direction == -1)
                        temp[ELEVATOR_WIDTH * j + 7] = 'v';
                }
            }
            strcat(snd, temp);     strcat(snd, "\n");

            strcat(snd, ELEVATOR_EMPTY_BODY);   strcat(snd, "floor calls:\n");
            strcat(snd, ELEVATOR_EMPTY_BODY);

            // print waiting floor calls
            if ((floor = shmat(shmFloorID[i-1], NULL, 0)) == (void *) -1) {
                fprintf(stderr, "[Client %d] [ERR_INFO]\n", pid);
                fprintf(stderr, "  Error: shmat(floor[%d])\n\n", i-1);
                goto F_EXIT;
            }

            passengerList_length = floor->passengers.length;
            print_length = (passengerList_length >= 10) ? 10 : passengerList_length;
            for (j = 0; j < print_length; j++) {
                memset(temp, 0, BUF_SIZE);
                sprintf(temp, " %d", floor->passengers.nodes[j].lift_call.floor);
                strcat(snd, temp);
            }
            shmdt(floor);

            if (passengerList_length - print_length > 0) {
                memset(temp, 0, BUF_SIZE);
                sprintf(temp, " ...(%d more)", passengerList_length - print_length);
                strcat(snd, temp);
            }
            strcat(snd, "\n");

            // print floor's tail
            strcat(snd, FLOOR_TAIL);
        }

        /* Write to clients */
        if (write(connfd, snd, BUF_SIZE) == -1) {
            fprintf(stderr, "[Client %d] [ERR_INFO]\n", pid);
            fprintf(stderr, "  Error: write() to connfd.\n\n");
            goto F_EXIT;
        }

        /* Detach the shared memory segments */
        for (i = 0; i < NUM_OF_ELEVATOR; i++)
            shmdt(elevatorPointerGroup[i]);

        time += 0.1;
        usleep(0.1 * 1e06);
    }

F_EXIT:
    /* Detach all shared memory segments */
    for (i = 0; i < NUM_OF_ELEVATOR; i++)
        shmdt(elevatorPointerGroup[i]);
    shmdt(floor);

    printf("[Client %d] [INFO]\n  Leaving...\n\n", pid);
    exit(0);
}


void gpioProcess() {
    const int bufsize = 2, lednum = 15, pathsize = 20;

    int fd_led[lednum];
    int fd_btn;

    char led[lednum][pathsize];
    int l = 0;

    /* Link the path of LED devices and open all devices*/
    char path_led[20] = "/dev/LED_00";
    char path_btn[12] = "/dev/BTN_00";

    // LEDs (15 GPIO)
    while (l < lednum) {
        path_led[9] = l/10 + '0';
        path_led[10] = l%10 + '0';

        snprintf(led[l], sizeof(led[l]), "%s", path_led);
        
        fd_led[l] = open(led[l], O_RDWR);
        if (fd_led[l] < 0) {
            printf("Error opening %s\n", path_led);
            exit(EXIT_FAILURE);
        }
        l++;
    }

    /*setup /dev path for buttons*/
    fd_btn = open(path_btn, O_RDWR);
    if (fd_btn < 0) {
        printf("Error opening /dev/BTN_00\n");
        exit(EXIT_FAILURE);
    }

        /*setup pid numbers*/
    int pids[9];
    int *shm_pid;
    if (sem_acquire(semaphorePid) < 0) {
        fprintf(stderr, "[gpio pid] Error: sem_acquire()\n");
        goto F_ERR_SEM;
    }
    if ((shm_pid = shmat(shmPidID, NULL, 0)) == (void *) -1) {
        fprintf(stderr, "[gpio pid] Error: shmat()\n");
        goto F_ERR_SHM;
    }
    for(int k = 0; k < 9; k++){
        pids[k] = shm_pid[k];
    }
    shmdt(shm_pid);
    sem_release(semaphorePid);


    int last_state = 0;
    while(1){
        Elevator *elevator[NUM_OF_ELEVATOR];
        char buf[bufsize];
        int rec[1] = {0};
        memset(buf, 0, bufsize);

        int i = 0;
        int current_floor; //1~5

        int *elevator_num;

        /*setup device driver*/
        for(i = 0; i < 3; i++){
            /* Attach the shared memory segment */
            if ((elevator[i] = shmat(shmElevatorID[i], NULL, 0)) == (void *) -1) {
                fprintf(stderr, "[elevatorSortFloorCallProcess] Error: shmat()\n");
                goto F_ERR_SHM;
            }
        }

        /*operation*/
        for(i = 0; i < NUM_OF_ELEVATOR; i++){
            memset(buf, sizeof(buf), 0);

            printf("for loop \n");
            /* Acquire the semaphore */
            if (sem_acquire(semaphoreElevator[i]) < 0) {
                fprintf(stderr, "[elevatorGpioProcess] Error: sem_acquire()\n");
                goto F_ERR_SEM;
            }
            current_floor = elevator[i]->current_floor;
            /* Release the semaphore */
            sem_release(semaphoreElevator[i]);

            for(int f = 0; f < NUM_OF_FLOOR; f++){
                
                int led_num = (f + i*5);
                // printf("\n\n>>> [for loop] check elevator num %d, F%d (corresponding led: %d)\n", i, f, led_num);
                if(f == current_floor-1){
                    buf[0] = 1;
                }else{
                    buf[0] = 0;
                }
                buf[1] = '\0';
                // printf(">>> [write to led %d: ] %d\n", led_num, buf);
                write(fd_led[led_num], buf, sizeof(buf));
            }

        }

        read(fd_btn, rec, sizeof(rec));
        int status = (last_state - rec[0]);
        last_state = rec[0];
        printf("required value = %d, turn status = %d\n", rec[0], last_state);

        for(int n = 0; n<NUM_OF_ELEVATOR; n++){
            if (sem_acquire(semaphoreElevator[n]) < 0) {
                fprintf(stderr, "[elevatorGpioProcess] Error: sem_acquire()\n");
                goto F_ERR_SEM;
            }
            if(status == -1){
                printf(">>> [GPIO] elevatorr turn into abnormal\n");
                elevator[n]->abnormal = 1;
            }else if(status == 1){

            printf(">>> [GPIO] elevator return normal\n");
                elevator[n]->abnormal = 0;
            }
            sem_release(semaphoreElevator[n]);
        }

        /*cleanup*/


        /* Detach the shared memory segment */
        for(i = 0; i < 3; i++){
            shmdt(elevator[i]);
        }
    }

F_ERR_SHM:
    sem_release(semaphoreElevator[0]);
    sem_release(semaphoreElevator[1]);
    sem_release(semaphoreElevator[2]);
F_ERR_SEM:
F_ERR_EXIT:
    exit(0);
}



/*************************** scheduler function set ***************************/

void reassignPassagePriority(Elevator *elevator) {
    int i;
    for (i = 0; i < elevator->callSequence.length; i++) {
        Call temp_call = elevator->callSequence.nodes[i];

        // We only adjust the priority of non up-peak calls
        if (!temp_call.isSpecialCall) {

            if (elevator->direction == 1) {
                // P1 : the call is the same direction (up) as the elevator
                //      and is higher than the elevator's current floor
                if ((temp_call.floor > elevator->current_floor) && (temp_call.direction == elevator->direction))
                    elevator->callSequence.nodes[i].passage = 1;
                // P3 : the call is the same direction (up) as the elevator
                //      and is lower than the elevator's current floor
                else if ((temp_call.floor < elevator->current_floor) && (temp_call.direction == elevator->direction))
                    elevator->callSequence.nodes[i].passage = 3;
                // P2 : the call has opposite direction to the elevator
                else
                    elevator->callSequence.nodes[i].passage = 2;
            }
            else if (elevator->direction == -1) {
                // P1 : the call is the same direction (down) as the elevator
                //      and is lower than the elevator's current floor
                if ((temp_call.floor < elevator->current_floor) && (temp_call.direction == elevator->direction))
                    elevator->callSequence.nodes[i].passage = 1;
                // P3 : the call is the same direction (down) as the elevator
                //      and is higher than the elevator's current floor
                else if ((temp_call.floor > elevator->current_floor) && (temp_call.direction == elevator->direction))
                    elevator->callSequence.nodes[i].passage = 3;
                // P2 : the call has opposite direction to the elevator
                else
                    elevator->callSequence.nodes[i].passage = 2;
            }
        }
    }

    elevatorCallSequenceSorting(elevator);
}


void elevatorCallSequenceSorting(Elevator *elevator) {
    int i, j;
    Call temp;

    for (i = 0; i < elevator->callSequence.length - 1; i++) {
        for (j = 1; j < elevator->callSequence.length; j++) {
            if (compareCalls(elevator->callSequence.nodes[i], elevator->callSequence.nodes[i]) == 1) {
                temp = elevator->callSequence.nodes[i];
                elevator->callSequence.nodes[i] = elevator->callSequence.nodes[j];
                elevator->callSequence.nodes[j] = temp;
            }
        }
    }
}


void elevatorAppendPassengerCall(Elevator *elevator, Passenger p) {
    Call new_floor_call = p.floor_call;
    Call new_lift_call = p.lift_call;

    insertEndLinkedCall(&(elevator->floorCalls), new_floor_call);
    insertEndLinkedCall(&(elevator->liftCalls), new_lift_call);
}


void elevatorOperationSleep(const unsigned long elevatorID, const double sleep_time) {
    /* Release the semaphore */
    sem_release(semaphoreElevator[elevatorID]);

    usleep(sleep_time * 1e06);

    /* Acquire the semaphore */
    if (sem_acquire(semaphoreElevator[elevatorID]) < 0) {
        fprintf(stderr, "[Elevator %ld] [ERR_INFO]\n  Error: sem_acquire()\n\n", elevatorID);
    }
}


/*************************** passenger-generating function set ***************************/

int generateSingleFloorPassenger(const int floor_ID, const unsigned int number_passenger) {
    int i;
    int current_floor = floor_ID, target_floor;
    int direction;
    unsigned long passenger_id;

    // shared memory segment
    unsigned long *shm_passenger_id;
    Floor *shm_floor;

    // temp
    Call new_floor_call, new_lift_call;
    Passenger new_passenger;

    /* Randomly create a passenger and append at the end of queue */
    for (i = 0; i < number_passenger; i++) {
        do {
            target_floor = rand() % NUM_OF_FLOOR + 1;
        } while (target_floor == current_floor);
        
        direction = (target_floor - current_floor) / abs(target_floor - current_floor);

        /****** PassengerID criteria ******/
        if (sem_acquire(semaphorePassengerID) < 0) {
            fprintf(stderr, "[Input System] [ERR_INFO]\n  Error: sem_acquire(passengerID)\n\n");
            goto F_ERR;
        }
        if ((shm_passenger_id = shmat(shmPassengerID_ID, NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Input System] [ERR_INFO]\n  Error: shmat(passengerID)\n");
            goto F_ERR;
        }

        passenger_id = (*shm_passenger_id)++;
        
        shmdt(shm_passenger_id);
        sem_release(semaphorePassengerID);
        /*********************************/

        new_floor_call = initCall(passenger_id, 1, 0, 1, current_floor, direction);
        new_lift_call = initCall(passenger_id, 0, 0, 1, target_floor, direction);
        new_passenger = initPassenger(passenger_id, new_floor_call, new_lift_call);

        /****** Floor criteria ******/
        if (sem_acquire(semaphoreFloor[floor_ID - 1]) < 0) {
            fprintf(stderr, "[Input System] [ERR_INFO]\n  Error: sem_acquire(floor[%d])\n\n", floor_ID - 1);
            goto F_ERR;
        }
        if ((shm_floor = shmat(shmFloorID[floor_ID - 1], NULL, 0)) == (void *) -1) {
            fprintf(stderr, "[Input System] [ERR_INFO]\n  Error: shmat(floor[%d])\n", floor_ID - 1);
            goto F_ERR;
        }

        insertEndLinkedPassenger(&(shm_floor->passengers_unchecked), new_passenger);

        shmdt(shm_floor);
        sem_release(semaphoreFloor[floor_ID - 1]);
        /****************************/
    }

    return 0;

F_ERR:
    return -1;
}

void generateAverageFloorPassenger(unsigned int number_passenger) {
    int division = number_passenger / NUM_OF_FLOOR;
    int remainder = number_passenger % NUM_OF_FLOOR;
    int i;

    for (i = 1; i <= NUM_OF_FLOOR; i++) {
        if (i <= remainder)
            generateSingleFloorPassenger(i, division + 1);
        else
            generateSingleFloorPassenger(i, division);
    }
}

void generateUpStairsPassenger(unsigned int number_passenger) {
    generateSingleFloorPassenger(1, number_passenger);
}

void generateDownStairsPassenger(unsigned int number_passenger) {
    int i;
    int den = 0;

    for (i = 1; i <= NUM_OF_FLOOR-1; i++)
        den += i;
    
    for (i = 1; i <= NUM_OF_FLOOR-1; i++)
        generateSingleFloorPassenger(i+1, number_passenger * i / den);
}
