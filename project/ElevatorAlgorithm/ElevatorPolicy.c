#include "ElevatorPolicy.h"

int policyRoundRobin(Elevator elevatorGroup[]) {
    static int count = 0;

    int select = (count++) % NUM_OF_ELEVATOR;

    // Find the first elevator whose capacity is not reached
    while (elevatorGroup[select].load >= elevatorGroup[select].capacity) {
        select = (count++) % NUM_OF_ELEVATOR;
    }
        

    return select;
}

int policyThreePassage(Elevator elevatorGroup[]) {
    int select = 0;
    int min_cost = INT_MAX, elevator_cost;

    int i, number_calls;
    Elevator temp_elevator;

    // Find the elevator with lowest cost
    for (i = 0; i < NUM_OF_ELEVATOR; i++) {
        temp_elevator = elevatorGroup[i];
        
        number_calls = temp_elevator.callSequence.length;
        // elevator_cost = (number_calls + 1) * 
        //     (temp_elevator.velocity * FLOOR_HEIGHT + 
        //     temp_elevator.passengerLoadingTime + temp_elevator.passengerUnloadingTime);
        elevator_cost = (number_calls + 1);

        if (elevator_cost < min_cost) {
            min_cost = elevator_cost;
            select = i;
        }
    }

    return select;
}


int policyZoning(int current_floor) {
    int zone = (int) ceil((double) NUM_OF_FLOOR / NUM_OF_ELEVATOR);
    int select = (int) floor((double) (current_floor - 1) / zone);

    return select;
}