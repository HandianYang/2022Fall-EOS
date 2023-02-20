#include "ElevatorProperty.h"

Call initCall(unsigned long ID, int isFloorCall, int isSpecialCall, int passage, int floor, int direction) {
    Call ret;
    ret.ID = ID;
    ret.isFloorCall = isFloorCall;
    ret.isSpecialCall = isSpecialCall;
    ret.passage = passage;
    ret.floor = floor;
    ret.direction = direction;

    return ret;
}

int compareCalls(Call new_call, Call origin_call) {

    /* First compare two calls by identifying whether they are special calls or not */
    if (new_call.isSpecialCall && origin_call.isSpecialCall)
        return 0;
    else if (new_call.isSpecialCall)
        return -1;
    else if (origin_call.isSpecialCall)
        return 1;
    
    /* Next compare two calls by identifying their passage priority */
    if (new_call.passage < origin_call.passage)
        return -1;
    else if (new_call.passage > origin_call.passage)
        return 1;
    else {
        /* Situations which the lower floor call should be served first
         *
         * Two possible situations:
         *  (1) The elevator goes up, with passage priority of call P1 or P3
         *  (2) The elevator goes down, with passage priority of call P2
         */
        if (new_call.direction == 1) {
            if (new_call.floor < origin_call.floor)
                return -1;
            else if (new_call.floor > origin_call.floor)
                return 1;
            else {  // if target floors of two calls are the same
                /* Next compare two calls with their type */
                if (!new_call.isFloorCall && origin_call.isFloorCall)
                    return -1;
                else if (new_call.isFloorCall && !origin_call.isFloorCall)
                    return 1;
            }
            return 0;
        }
        /* Situations which the higher floor call should be served first
         *
         * Two possible situations:
         *  (1) The elevator goes down, with passage priority of call P1 or P3
         *  (2) The elevator goes up, with passage priority of call P2
         */
        else if (new_call.direction == -1) {
            if (new_call.floor > origin_call.floor)
                return -1;
            else if (new_call.floor < origin_call.floor)
                return 1;
            else {  // if target floors of two calls are the same
                /* Next compare two calls with their type */
                if (!new_call.isFloorCall && origin_call.isFloorCall)
                    return -1;
                else if (new_call.isFloorCall && !origin_call.isFloorCall)
                    return 1;
            }
            return 0;
        }
    }
}

void insertBeginLinkedCall(CallList *list, Call call) {
    int i;
    for (i = list->length; i > 0; i--)
        list->nodes[i] = list->nodes[i - 1];
    
    list->nodes[0] = call;
    list->length++;
}

void insertEndLinkedCall(CallList *list, Call call) {
    list->nodes[list->length++] = call;
}

void insertPriorityCall(CallList *list, Call call) {
    int i, j;
    unsigned short isDone = 0;

    for (i = 0; i < list->length; i++) {
        if (compareCalls(call, list->nodes[i]) == -1) {
            for (j = list->length; j > i; j--)
                list->nodes[j] = list->nodes[j - 1];
            
            list->nodes[i] = call;
            list->length++;
            return;
        }
    }

    list->nodes[(list->length)++] = call;
}

Call deleteBeginLinkedCall(CallList *list) {
    if (isEmptyCallList(*list)) {
        fprintf(stderr, "[deleteBeginLinkedCall()] Error: cannot delete calls from an empty list! Return NULL_CALL instead.\n");
        return NULL_CALL; // cannot delete an empty list
    }

    Call temp = list->nodes[0];
    int i;
    for (i = 0; i < list->length - 1; i++)
        list->nodes[i] = list->nodes[i + 1];
    
    list->nodes[--(list->length)] = NULL_CALL;

    return temp; // return the deleted node
}

Call deleteEndLinkedCall(CallList *list) {
    if (isEmptyCallList(*list)) {
        fprintf(stderr, "[deleteEndLinkedCall()] Error: cannot delete elements from an empty list. Return NULL_CALL instead.\n");
        return NULL_CALL; // cannot delete an empty list
    }

    Call temp = list->nodes[--(list->length)];
    list->nodes[list->length] = NULL_CALL;
    return temp; // return the deleted node
}

Call deleteIndexLinkedCall(CallList *list, unsigned int index) {
    if (isEmptyCallList(*list)) {
        fprintf(stderr, "[deleteIndexLinkedCall()] Error: cannot delete elements from an empty list. Return NULL_CALL instead.\n");
        return NULL_CALL; // cannot delete an empty list
    }
    if (index >= list->length) {
        fprintf(stderr, "[deleteIndexLinkedCall()] Error: index %d should be smaller than the length of linked-list (%d).", index, list->length);
        fprintf(stderr, " Return NULL_CALL instead.\n");
        return NULL_CALL;
    }
    if (index < 0) {
        fprintf(stderr, "[deleteIndexLinkedCall()] Error: index %d should be larger than 0.", index);
        fprintf(stderr, " Return NULL_CALL instead.\n");
        return NULL_CALL;
    }

    Call temp = list->nodes[index];
    int i;
    for (i = index; i < list->length - 1; i++) {
        list->nodes[i] = list->nodes[i + 1];
    }
    list->length--;

    return temp; // return the deleted node
}

Call deleteIDLinkedCall(CallList *list, unsigned long id) {
    if (isEmptyCallList(*list)) {
        fprintf(stderr, "[deleteIDLinkedCall()] Error: cannot delete elements from an empty list. Return NULL_CALL instead.\n");
        return NULL_CALL; // cannot delete an empty list
    }

    int i;
    for (i = 0; i < list->length; i++) {
        if (list->nodes[i].ID == id)
            return deleteIndexLinkedCall(list, i);
    }

    fprintf(stderr, "[deleteIDLinkedCall()] Error: cannot find call with ID=%ld. Return NULL_CALL instead.\n", id);
    return NULL_CALL;
}

Call getIndexLinkedCall(CallList list, unsigned int index) {
    if (isEmptyCallList(list)) {
        fprintf(stderr, "[getIndexLinkedCall()] Error: cannot get elements from an empty list. Return NULL_CALL instead.\n");
        return NULL_CALL; // cannot delete an empty list
    }
    if (list.length <= index) {
        fprintf(stderr, "[getIndexLinkedCall()] Error: index %d should be smaller than the length of linked-list (%d).", index, list.length);
        fprintf(stderr, " Return NULL_CALL instead.\n");
        return NULL_CALL;
    }
    if (index < 0) {
        fprintf(stderr, "[getIndexLinkedCall()] Error: index %d should be larger than 0.", index);
        fprintf(stderr, " Return NULL_CALL instead.\n");
        return NULL_CALL;
    }

    return list.nodes[index];
}

void printCall(Call call) {
    printf("  Node:\n");
    printf("    - ID number = %ld\n", call.ID);
    printf("    - isFloorCall = %d\n", call.isFloorCall);
    printf("    - isSpecialCall = %d\n", call.isSpecialCall);
    printf("    - passage priority = %d\n", call.passage);
    printf("    - floor number = %d\n", call.floor);
    printf("    - direction = %d\n", call.direction);
}

void printCallList(CallList list) {
    unsigned short i = 0;

    printf("The length of the list: %d\n", list.length);
    for (i = 0; i < list.length; i++) {
        printf("  Node %d:\n", i);
        printf("    - ID number = %ld\n", list.nodes[i].ID);
        printf("    - isFloorCall = %d\n", list.nodes[i].isFloorCall);
        printf("    - isSpecialCall = %d\n", list.nodes[i].isSpecialCall);
        printf("    - passage priority = %d\n", list.nodes[i].passage);
        printf("    - floor number = %d\n", list.nodes[i].floor);
        printf("    - direction = %d\n", list.nodes[i].direction);
    }
    printf("End\n\n");
}

int isEmptyCallList(CallList list) {
    return (list.length == 0);
}

/********************************** PassengerList **********************************/

Passenger initPassenger(unsigned long ID, Call floor_call, Call lift_call) {
    Passenger ret;
    ret.ID = ID;
    ret.floor_call = floor_call;
    ret.lift_call = lift_call;

    return ret;
}


void insertBeginLinkedPassenger(PassengerList *list, Passenger passenger) {
    int i;
    for (i = list->length; i > 0; i--)
        list->nodes[i] = list->nodes[i - 1];
    
    list->nodes[0] = passenger;
    list->length++;
}


void insertEndLinkedPassenger(PassengerList *list, Passenger passenger) {
    list->nodes[list->length++] = passenger;
}


Passenger deleteBeginLinkedPassenger(PassengerList *list) {
    if (isEmptyPassengerList(*list)) {
        fprintf(stderr, "[deleteBeginLinkedPassenger()] Error: cannot delete passengers from an empty list. Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER; // cannot delete an empty list
    }

    Passenger temp = list->nodes[0];
    int i;
    for (i = 0; i < list->length - 1; i++)
        list->nodes[i] = list->nodes[i + 1];

    list->nodes[--(list->length)] = NULL_PASSENGER;

    return temp; // return the deleted node
}


Passenger deleteEndLinkedPassenger(PassengerList *list) {
    if (isEmptyPassengerList(*list)) {
        fprintf(stderr, "[deleteEndLinkedPassenger()] Error: cannot delete passengers from an empty list. Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER; // cannot delete an empty list
    }

    Passenger temp = list->nodes[--(list->length)];
    list->nodes[list->length] = NULL_PASSENGER;
    return temp; // return the deleted node
}


Passenger deleteIndexLinkedPassenger(PassengerList *list, unsigned int index) {
    if (isEmptyPassengerList(*list)) {
        fprintf(stderr, "[deleteIndexLinkedPassenger()] Error: cannot delete elements from an empty list. Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER;  // cannot delete an empty list
    }

    if (index >= list->length) {
        fprintf(stderr, "[deleteIndexLinkedPassenger()] Error: index %d should be smaller than the length of linked-list (%d).", index, list->length);
        fprintf(stderr, " Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER;  // index is out of bound
    }
    if (index < 0) {
        fprintf(stderr, "[deleteIndexLinkedPassenger()] Error: index %d should be larger than or equal to 0.", index);
        fprintf(stderr, " Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER;
    }

    Passenger temp = list->nodes[index];
    int i;
    for (i = index; i < list->length - 1; i++)
        list->nodes[i] = list->nodes[i + 1];
    
    list->length--;

    return temp; // return the deleted node
}


Passenger deleteIDLinkedPassenger(PassengerList *list, unsigned long id) {
    if (isEmptyPassengerList(*list)) {
        fprintf(stderr, "[deleteIDLinkedPassenger()] Error: cannot delete elements from an empty list. Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER; // cannot delete an empty list
    }

    int i;
    for (i = 0; i < list->length; i++) {
        if (list->nodes[i].ID == id)
            return deleteIndexLinkedPassenger(list, i);
    }

    fprintf(stderr, "[deleteIDLinkedPassenger()] Error: cannot find call with ID=%ld. Return NULL_PASSENGER instead.\n", id);
    return NULL_PASSENGER;
}


Passenger getIndexLinkedPassenger(PassengerList list, unsigned int index) {
    if (isEmptyPassengerList(list)) {
        fprintf(stderr, "[getIndexLinkedPassenger()] Error: cannot get elements from an empty list. Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER; // cannot delete an empty list
    }

    if (index >= list.length) {
        fprintf(stderr, "[getIndexLinkedPassenger()] Error: index %d should be smaller than the length of linked-list (%d).", index, list.length);
        fprintf(stderr, " Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER;
    }
    if (index < 0) {
        fprintf(stderr, "[getIndexLinkedPassenger()] Error: index %d should be larger than or equal to 0.", index);
        fprintf(stderr, " Return NULL_PASSENGER instead.\n");
        return NULL_PASSENGER;
    }

    return list.nodes[index];
}


void printPassenger(Passenger passenger) {
    printf("[printPassenger] Passenger's ID = %ld\n", passenger.ID);
    printf("Passenger's floor call: \n\n");
    printCall(passenger.floor_call);
    printf("Passenger's lift call: \n\n");
    printCall(passenger.lift_call);
}


void printPassengerList(PassengerList list) {
    int i;

    printf("[printPassengerList] The length of the list: %d\n\n", list.length);
    for (i = 0; i < list.length; i++) {
        printPassenger(list.nodes[i]);
    }
}


int isEmptyPassengerList(PassengerList list) {
    return (list.length == 0);
}