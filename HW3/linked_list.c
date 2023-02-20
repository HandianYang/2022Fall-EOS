#include "linked_list.h"

int isEmptyList(List list) {
    return (list.length == 0);
}

Node initNode(int region_number, int severe_case, int mild_case, double execution_time) {
    Node ret;
    ret.region_number = region_number;
    ret.severe_case = severe_case;
    ret.mild_case = mild_case;
    ret.execution_time = execution_time;

    return ret;
}

void insertBegin(List *list, Node node) {
    int i;
    for (i = list->length; i > 0; i--) {
        list->nodes[i] = list->nodes[i-1];
    }
    list->nodes[0] = node;
    list->length++;
}

void insertEnd(List *list, Node node) {
    list->nodes[list->length] = node;
    list->length++;
}

Node deleteBegin(List *list) {
    if (isEmptyList(*list)) {
        fprintf(stderr, "[deleteBegin()] Error: cannot delete elements from an empty list. Return NULL_NODE instead.\n");
        return NULL_NODE;    // cannot delete an empty list
    }

    Node temp = list->nodes[0];
    int i;
    for (i = 0; i < list->length - 1; i++) {
        list->nodes[i] = list->nodes[i+1];
    }
    list->nodes[--(list->length)] = ZERO_NODE;

    return temp;   // return the deleted node
}

Node deleteEnd(List *list) {
    if (isEmptyList(*list)) {
        fprintf(stderr, "[deleteEnd()] Error: cannot delete elements from an empty list. Return NULL_NODE instead.\n");
        return NULL_NODE;    // cannot delete an empty list
    }

    Node temp = list->nodes[--(list->length)];
    list->nodes[list->length] = ZERO_NODE;
    return temp;    // return the deleted node
}

void addNewCaseNumber(List *list, Node node) {
    unsigned short i, same_region_is_found = 0;
    
    for (i = 0; i < list->length; i++) {
        if (list->nodes[i].region_number == node.region_number) {
            list->nodes[i].severe_case += node.severe_case;
            list->nodes[i].mild_case += node.mild_case;
            same_region_is_found = 1;
            break;
        }
    }

    if (!same_region_is_found)
        insertEnd(list, node);

}


void printNode(Node node) {
    printf("Node:\n");
    printf("\t region = %d\n", node.region_number);
    printf("\t severe = %d\n", node.severe_case);
    printf("\t mild   = %d\n", node.mild_case);
    printf("\t time   = %f\n", node.execution_time);
    printf("\n");
}


void printList(List list) {
    unsigned short i = 0;
    printf("The length of linked-list: %d\n\n", list.length);
    for (i = 0; i < list.length; i++) {
        printf("Node %d:\n", i);
        printf("\t region = %d\n", list.nodes[i].region_number);
        printf("\t severe = %d\n", list.nodes[i].severe_case);
        printf("\t mild   = %d\n", list.nodes[i].mild_case);
        printf("\t time   = %f\n", list.nodes[i].execution_time);
    }
    printf("\n");
}

double getNodeWaitingTime(List list, Node node) {
    double time = 0;
    unsigned short i;

    // the node with same region number is the first node
    if (list.nodes[0].region_number == node.region_number) {
        time += (list.nodes[0].region_number - list.nodes[0].execution_time);
    }
    // the node has not been executed yet 
    else {
        time += node.region_number;
        for (i = 0; i < list.length; i++) {
            time += (list.nodes[i].region_number - list.nodes[i].execution_time);

            // If a node with same region number is found, then stop calculating time
            if (list.nodes[i].region_number == node.region_number)
                break;
        }        
    }

    return time;
}