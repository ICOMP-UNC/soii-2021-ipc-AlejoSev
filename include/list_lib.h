#ifndef _LIST_LIB_H_
#define _LIST_LIB_H_
#define PACKET_LENGTH 64

typedef struct node {
    char val[PACKET_LENGTH];
    struct node * next;
} node_t;

void print_list(node_t*);
void push_end(node_t*, char[]);
void push_beginning(node_t**, char[]);
char* pop(node_t**);
char* remove_last(node_t*);
char* remove_by_index(node_t**, int);
#endif