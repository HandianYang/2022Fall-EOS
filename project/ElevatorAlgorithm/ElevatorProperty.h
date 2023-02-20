#ifndef _ELEVATOR_PROPERTY_H_
#define _ELEVATOR_PROPERTY_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 10000
#define MAX_LENGTH 1000
#define DEBUG 0
#define NUM_OF_FLOOR 5
#define NUM_OF_ELEVATOR 3
#define FLOOR_HEIGHT 3          // unit: m

typedef struct Call {
    unsigned long ID;

    int isFloorCall;
    /*
     * [Value]
     *  1 : floor call
     *  0 : lift call
     */

    int isSpecialCall;
    /*
     * [Value]
     *  1 : up-peak call
     *  0 : non up-peak call
     */

    int passage;
    /*
     * [Value]
     *  1 - P1: floor call at a higher/lower floor with up/down direction
     *          when the elevator goes up/down
     *  2 - P2: floor call with opposite direction reference to the
     *          elevator
     *  3 - P3: floor call at a lower/higher floor with up/down direction
     *          when the elevator goes up/down
     * [Note] All requests are performed according to the order
     *          P1 -> P2 -> P3
     */
    
    int floor;
    /*
     * [Value]
     *  (int) : the floor from where the floor call invokes, or
     *          the floor to where the lift call requests
     */
    
    int direction;
    /*
     * [Value]
     *  -1 : down
     *   0 : idle
     *   1 : up
     */        

} Call;

/* Priority blocking queue (linked-list) of struct Call */
typedef struct CallList {
    Call nodes[MAX_LENGTH];     // priority blocking queue 
    int length;
} CallList;

/* Initialize the node for Call */
Call initCall(unsigned long ID, int isFloorCall, int isSpecialCall, int passage, int floor, int direction);
#define NULL_CALL initCall(0,0,0,0,0,0);

/* Compare the priority of two calls
 * 
 * [return]
 *  -1 : The new call goes before the origin call
 *   0 : The new call is equivalent to the origin call
 *   1 : The new call goes after the origin call
 */
int compareCalls(Call new_call, Call origin_call);

/* Insert call at the beginning of the list */
void insertBeginLinkedCall(CallList *list, Call call);

/* Insert call at the end of the list */
void insertEndLinkedCall(CallList *list, Call call);

/* Insert call according to its priority order */
void insertPriorityCall(CallList *list, Call call);

/* Delete the call at the beginning of the list */
Call deleteBeginLinkedCall(CallList *list);

/* Delete the call at the end of the list */
Call deleteEndLinkedCall(CallList *list);

/* Delete the call at some specific position of the list */
Call deleteIndexLinkedCall(CallList *list, unsigned int index);

/* Delete the call according to its ID number */
Call deleteIDLinkedCall(CallList *list, unsigned long id);

/* Obtain Call at the specific position of the list */
Call getIndexLinkedCall(CallList list, unsigned int index);

/* Print out one single node */
void printCall(Call call);

/* Print out the overall list */
void printCallList(CallList list);

/* Check if the queue is empty */
int isEmptyCallList(CallList list);


/**
 * A passenger always has to invoke both floor call and lift call
 */
typedef struct Passenger {
    unsigned long ID;   // unique number
    Call floor_call;    // Calls from the outside of the elevators (from which floor?)
    Call lift_call;     // Calls inside the elevators (to which floor?)
} Passenger;


typedef struct PassengerList {
    Passenger nodes[MAX_LENGTH];    // linked-list of passengers
    int length;
} PassengerList;

/* Initialize the node for Passenger */
Passenger initPassenger(unsigned long ID, Call floor_call, Call lift_call);
#define NULL_PASSENGER initPassenger(0, initCall(0,0,0,0,0,0), initCall(0,0,0,0,0,0));

/* Insert passenger at the beginning of the list */
void insertBeginLinkedPassenger(PassengerList *list, Passenger passenger);

/* Insert passenger at the end of the list */
void insertEndLinkedPassenger(PassengerList *list, Passenger passenger);

/* Delete the passenger at the beginning of the list */
Passenger deleteBeginLinkedPassenger(PassengerList *list);

/* Delete the passenger at the end of the list */
Passenger deleteEndLinkedPassenger(PassengerList *list);

/* Delete the passenger at some specific position of the list */
Passenger deleteIndexLinkedPassenger(PassengerList *list, unsigned int index);

/* Delete the passenger according to its ID number */
Passenger deleteIDLinkedPassenger(PassengerList *list, unsigned long id);

/* Obtain Call at the specific position of the list */
Passenger getIndexLinkedPassenger(PassengerList list, unsigned int index);

/* Print out one single node */
void printPassenger(Passenger passenger);

/* Print out the overall list */
void printPassengerList(PassengerList list);

/* Check if the queue is empty */
int isEmptyPassengerList(PassengerList list);

typedef struct Floor {
    unsigned long ID;                   // 1 ~ NUM_OF_FLOOR
    PassengerList passengers;           // Holds passengers at this floor
    PassengerList passengers_unchecked; // First add new passengers to this list
} Floor;

typedef struct Elevator {
    unsigned long ID;       // 0 ~ NUM_OF_ELEVATOR - 1

    CallList floorCalls;    // linked-list of floor calls
    CallList liftCalls;     // linked-list of lift calls
    CallList callSequence;  // priority blocking queue of all Calls

    int load;               // current passengers in elevator
    int algorithm;          // Set at the time of elevator creation
    int current_floor;
    int direction;          // 1 - Up, -1 - Down, 0 - idle
    int idle;               // 1 - idle
    int abnormal;           // 1 - abnormal (interrupt), 0 - normal status  //[e'0104]

    double passenger_loading_time;      // unit: sec/person
    double passenger_unloading_time;    // unit: sec/person
    double velocity;                    // unit: m/s
    unsigned long capacity;             // unit: person
} Elevator;

#endif
