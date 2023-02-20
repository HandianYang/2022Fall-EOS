#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include <stdio.h>
#include <stdlib.h>

#define MAX_LENGTH 100

typedef struct Node {
    int region_number;
    int severe_case;
    int mild_case;
    double execution_time;
} Node;

typedef struct List {
    Node nodes[MAX_LENGTH];
    unsigned int length;
} List;

/* Check if the linked-list is empty */
int isEmptyList(List list);

/* Initialize one node */
Node initNode(int region_number, int severe_case, int mild_case, double execution_time);
#define NULL_NODE initNode(-1,-1,-1,-1);
#define ZERO_NODE initNode(0,0,0,0);

/* Insert at the beginning of linked-list */
void insertBegin(List *list, Node node);

/* Insert at the end of linked-list */
void insertEnd(List *list, Node node);

/* Delete the begin node of linked-list */
Node deleteBegin(List *list);

/* Delete the end node of linked-list */
Node deleteEnd(List *list);

/* Add new reported case into linked-list */
void addNewCaseNumber(List *list, Node node);

/* Print out one single node */
void printNode(Node node);

/* Print out the overall linked-list */
void printList(List list);

/* Obtain current waiting time for one node */
double getNodeWaitingTime(List list, Node node);

#endif