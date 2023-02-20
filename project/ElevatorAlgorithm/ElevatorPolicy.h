#ifndef _ELEVATOR_POLICY_H_
#define _ELEVATOR_POLICY_H_

#include <limits.h>
#include <math.h>
#include "ElevatorProperty.h"

/**
 * Round-Robin Group Elevator Scheduling.
 * First-come-first-served approach.
 *
 * The goal is to achieve an equal load for all cars.
 *
 * Calls are assigned in the order they arrive in a sequential way to single elevators.
 * Call 0 is assigned to car 0, call 1 to car 1, . . . call L to car L, call L + 1 to car 0, and so on.
 */
int policyRoundRobin(Elevator elevatorGroup[]);

/**
 * Three Passage Group Elevator Scheduling.
 *
 * Estimates the costs that would result from assigning the new call to the elevator.
 *
 * Stop costs are static and include the period of time necessary for opening the door,
 * unloading and loading each one passenger and closing the door.
 *
 * In case every elevator already reached 80% load (number of calls > 80% elevator capacity),
 * calls will not get assigned until at least one elevator falls below this mark.
 *
 * The call is assigned to the elevator with the lowest costs.
 */
int policyThreePassage(Elevator elevatorGroup[]);

/**
 * Zoning Group Elevator Scheduling.
 *
 * Splits a building into several adjacent zones.
 *
 * Every elevator only serving floor calls that occur in the zone assigned to the respective car.
 * The primary objective is to reduce the number of car stops and therefore the total journey time.
 *
 * A building served by L elevators can be split into up to L zones, where these m zones are either disjoint or not.
 * Cars in idle state are repositioned to the zone’s center level, therefore minimizing waiting time for passengers on adjacent floors.
 *
 * Static Zoning, zones are assigned permanently to a group of elevators.
 */
int policyZoning(int current_floor);


#endif