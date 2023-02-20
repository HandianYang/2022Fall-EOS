#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ElevatorAlgorithm/ElevatorProperty.h"
#include "ElevatorAlgorithm/ElevatorPolicy.h"
#include "ElevatorAlgorithm/ElevatorOperation.h"
#include "ElevatorAlgorithm/InterProcessCommunication.h"

/* main function */
int main(int argc, char *argv[])
{
    if (argc != 3)
        errexit("Usage: %s <port> <algorithm>\n", argv[0]);
    
    printf("****************************\n");
    printf("*      DEBUG mode: %s     *\n", DEBUG ? "ON " : "OFF");
    printf("****************************\n");    

    /* Set random seeds */
    srand(time(NULL));

    /* Register important signals */
    if (signal(SIGINT, SIGINT_handler) == SIG_ERR) {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to caught SIGINT signal.\n\n");
        goto F_ERR_SIGNAL;
    }
    if (signal(SIGCHLD, SIGCHLD_handler) == SIG_ERR) {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to caught SIGCHLD signal.\n\n");
        goto F_ERR_SIGNAL;
    }
        
    int retval, algorithm = atoi(argv[2]);

    /* Initialize semaphores */
    retval = semaphoreInit();
    if (retval < 0) {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to initialize all semaphores.\n\n");
        goto F_ERR_SEM_INIT;
    }

    /* Initialize shared memory */
    retval = sharedMemoryInit(algorithm);
    if (retval < 0) {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to initialize all shared memory segments.\n\n");
        goto F_ERR_SHM_INIT;
    }

    /* Create all elevator processes */
    pid_t elevator_simluation_pid[NUM_OF_ELEVATOR];
    pid_t elevator_assign_floor_call_pid[NUM_OF_ELEVATOR];
    pid_t elevator_up_peak_pid[NUM_OF_ELEVATOR];
    pid_t elevator_zoning_pid[NUM_OF_ELEVATOR];
    pid_t elevator_scheduler_pid;
    pid_t gpio_pid;
    pid_t parent_pid;

    int i;
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        // create elevatorSimulationProcess
        if ((elevator_simluation_pid[i] = fork()) == 0)
            elevatorSimulationProcess(i);
        else if (elevator_simluation_pid[i] < 0) {
            fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create elevatorSimulationProcess(%d).\n\n", i);
            goto F_ERR_PROCESS;
        }

        // create elevatorAssignFloorCallProcess
        if ((elevator_assign_floor_call_pid[i] = fork()) == 0)
            elevatorAssignFloorCallProcess(i);
        else if (elevator_assign_floor_call_pid[i] < 0) {
            fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create elevatorAssignFloorCallProcess(%d).\n\n", i);
            goto F_ERR_PROCESS;
        }

        // create elevatorUpPeakProcess
        if (algorithm == 2) {
            if ((elevator_up_peak_pid[i] = fork()) == 0)
                elevatorUpPeakProcess(i);
            else if (elevator_up_peak_pid[i] < 0) {
                fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create elevatorUpPeakProcess(%d).\n\n", i);
                goto F_ERR_PROCESS;
            }
        }

        // create elevatorZoningProcess
        if (algorithm == 4) {
            if ((elevator_zoning_pid[i] = fork()) == 0)
                elevatorZoningProcess(i);
            else if (elevator_zoning_pid[i] < 0) {
                fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create elevatorZoningProcess(%d).\n\n", i);
                goto F_ERR_PROCESS;
            }
        }
    }
    
    // create elevatorSchedulerProcess
    if ((elevator_scheduler_pid = fork()) == 0)
        elevatorSchedulerProcess(algorithm);
    else if (elevator_scheduler_pid < 0) {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create elevatorSchedulerProcess().\n\n");
        goto F_ERR_PROCESS;
    }

    // create gpioProcess [e]
    if ((gpio_pid = fork()) == 0)
        gpioProcess();
    else if (gpio_pid < 0)
    {
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create gpioProcess().\n\n");
        goto F_ERR_PROCESS;
    }

    /* Create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);

    // create parentProcess
    if ((parent_pid = fork()) == 0) {
        parentProcess(sockfd,addr_cln,sLen);
    }
    else if (parent_pid < 0) {
        //error
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to create parentProcess().\n\n");
        goto F_ERR_PROCESS;
    }

    // main menu
    int select;
    int num_people;
    int floor_id;

    while (1) {
MENU:
        printf("\n[Main Server] [INFO] \n  Menu: \n");
        printf("\t(1) Generate at specific floor\n");
        printf("\t(2) Generate at each floor evenly\n");
        printf("\t(3) Generate large number of ground-floor passengers\n");
        printf("\t(4) Generate large number of down-stairs passengers\n");
        printf("  Select one policy (or 0 to exit): ");
        scanf("%d", &select);

        if (select == 0) {
            goto F_EXIT;
        }
        else if (select == 1) {
            printf("\n[Main Server] [INFO] \n  (1) Generate at specific floor\n");
            printf("  Please enter the number of passengers: ");
            scanf("%d", &num_people);
            printf("  Please enter the floor number: ");
            scanf("%d", &floor_id);

            generateSingleFloorPassenger(floor_id, num_people);
        }
        else if (select == 2) {
            printf("\n[Main Server] [INFO] \n  (2) Generate at each floor evenly\n");
            printf("  Please enter the number of passengers: ");
            scanf("%d", &num_people);

            generateAverageFloorPassenger(num_people);
        }
        else if (select == 3) {
            printf("\n[Main Server] [INFO] \n  (3) Generate large number of up-stair passengers\n");
            printf("  Please enter the number of passengers: ");
            scanf("%d", &num_people);

            generateUpStairsPassenger(num_people);
        }
        else if (select == 4) {
            printf("\n[Main Server] [INFO] \n  (4) Generate large number of down-stair passengers\n");
            printf("  Please enter the number of passengers: ");
            scanf("%d", &num_people);

            generateDownStairsPassenger(num_people);
        }
        else {
            printf("\n[Main Server] [ERR_INFO] \n  Unsupported input %d.\n", select);
            goto MENU;
        }

        printf("[Main Server] [INFO] \n  SUCCESS!! Wait for 3 seconds for another commands...\n\n");
        sleep(3);
    }

F_EXIT:
    
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        kill(elevator_simluation_pid[i], SIGKILL);

        kill(elevator_assign_floor_call_pid[i], SIGKILL);

        if (algorithm == 2)
            kill(elevator_up_peak_pid[i], SIGKILL);
        
        if (algorithm == 4)
            kill(elevator_zoning_pid[i], SIGKILL);
    }
    kill(elevator_scheduler_pid, SIGKILL);
    // kill(gpio_pid, SIGKILL);
    kill(parent_pid, SIGKILL);

    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Release all semaphores */
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        sem_release(semaphoreElevator[i]);
    }
    for (i = 0; i < NUM_OF_FLOOR; i++) {
        sem_release(semaphoreFloor[i]);
    }
    sem_release(semaphorePassengerID);

    /* Destroy share memory segment */
    retval = sharedMemoryRemove();
    if (retval < 0)
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to remove all shared memory.\n\n");
    
    /* Remove semaphores */
    retval = semaphoreRemove();
    if (retval < 0)
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to remove all semaphores.\n\n");
    
    printf("[Main Server] [INFO] Leavnig...\n\n");
    return 0;

F_ERR_PROCESS:
    /* Disconnect the server and the client */
    close(sockfd);
    close(connfd);

    /* Destroy share memory segment */
    retval = sharedMemoryRemove();
    if (retval < 0)
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to remove all shared memory.\n\n");

F_ERR_SHM_INIT:
    /* Remove semaphores */
    retval = semaphoreRemove();
    if (retval < 0)
        fprintf(stderr, "[Main Server] [ERR_INFO]\n  Error: Failed to remove all semaphores.\n\n");
    
F_ERR_SEM_INIT:
F_ERR_SIGNAL:
    return -1;
}