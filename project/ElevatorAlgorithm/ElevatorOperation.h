#ifndef _ELEVATOR_OPERATION_H_
#define _ELEVATOR_OPERATION_H_

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fcntl.h> //[e]
#include <signal.h>
#include <sys/time.h>
#include "ElevatorProperty.h"
#include "ElevatorPolicy.h"
#include "InterProcessCommunication.h"

/*************************** Process function set ***************************/

/* Simulates the elevator moving through the shaft */
void elevatorSimulationProcess(const unsigned long elevatorID);

/* Sort and assign new floor calls into callSequence */
void elevatorAssignFloorCallProcess(const unsigned long elevatorID);

/* Schedule one elevator to serve one passenger */
void elevatorSchedulerProcess(const int algorithm);

/* Up-peak policy: Drive the elevator to the ground floor if idle */
/* [Note] only activated when the up-peak policy is chosen */
void elevatorUpPeakProcess(const unsigned long elevatorID);

/* Zoning policy: Drive the elevator to the zone origin floor if idle */
/* [Note] only activated when the zoning policy is chosen */
void elevatorZoningProcess(const unsigned long elevatorID);

extern struct timeval tv1;
extern struct timeval tv2;
/*Parent Process*/
void parentProcess(int sockfd,struct sockaddr_in addr_cln,socklen_t sLen);

/* Client process */
void clientProcess(const int connfd);

/* GPIO driver */
void gpioProcess();

/*************************** scheduler function set ***************************/

/* Re-assign the passage priority of all Calls in callSequence */
void reassignPassagePriority(Elevator *elevator);

/* Sort the callSequence of the elevator */
void elevatorCallSequenceSorting(Elevator *elevator);

/* Assign one passenger's call to the elevator */
void elevatorAppendPassengerCall(Elevator *elevator, Passenger p);


/*************************** simulation function set ***************************/

/* Helps elevators perform operations that take some time */
void elevatorOperationSleep(const unsigned long elevatorID, const double sleep_time);

#define FLOOR_HEAD " **********  **********  ********** "
#define FLOOR_TAIL "------------------------------------------------------------------------\n"
#define ELEVATOR_EMPTY_BODY " *        *  *        *  *        * "
#define ELEVATOR_WIDTH (sizeof(ELEVATOR_EMPTY_BODY) - 1) / 3

/*************************** passenger-generating function set ***************************/

/* Generate numbers of passengers at one specific floor */
int generateSingleFloorPassenger(const int floor_ID, const unsigned int number_passenger);

/* Generate numbers of passengers and evenly scatter them average at each floor */
void generateAverageFloorPassenger(unsigned int number_passenger);

/* Simulate that large numbers of passengers wait for going upstairs at the ground floor
 * 
 * Scanerio:
 *  (1) apartments at off-work hours (PM 5:00)
 *  (2) department stores at the beginning of opening hours (AM 11:00)
 *  (3) Engineer Building E of NYCU at PM 3:00 on every Wednesday
 */
void generateUpStairsPassenger(unsigned int number_passenger);

/* Simulate that large numbers of passengers wait for going downstairs
 * 
 * Scanerio:
 *  (1) apartments at on-duty hours (AM 7:00)
 *  (2) department stores at the end of opening hours (PM 9:50)
 */
void generateDownStairsPassenger(unsigned int number_passenger);

#endif
