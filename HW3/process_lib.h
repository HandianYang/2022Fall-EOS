#ifndef _PROCESS_LIB_H
#define _PROCESS_LIB_H

#include <signal.h>
#include <string.h>
#include <time.h> 
#include "IPC_lib.h"
#include "sockop.h"
#include "linked_list.h"

#ifndef DEBUG
#define DEBUG 1
#endif

/************************* client process function set *************************/

/* Clients operation process */
void clientProcess(const int connfd);

/* Obtain known case number of either 'Severe' or 'Mild' in the given region 
 * 
 * [return]
 *  (int) : case number
 *     -1 : error (cannot attach shared memory segment)
 */
int getConfirmedData(int region_num, int isSevere);

/* Record new known case number and store it into shared memory */
void addConfirmedData(int region_num, int isSevere, int num_case);


/************************* ambulance process function set *************************/

/* Ambulances working processes */
void ambulanceProcess(const int ID);

/* Operate ambulance scheduling and obtain the shortest waiting time */
double getAmbulanceWaitingTime(Node newNode);

/************************* signal registering function set *************************/

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum);

/* zombie process handler */
void zombie_process_handler(int signum);

#endif