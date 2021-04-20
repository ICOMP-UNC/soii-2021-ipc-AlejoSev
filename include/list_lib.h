#ifndef _LIST_LIB_H_
#define _LIST_LIB_H_
#define PACKET_LENGTH 128

struct Node {
    int fd;
    int address;
    struct Node* next;
};

void push_client(struct Node**, int, int);
void delete_client_by_address(struct Node**, int);
void delete_client_by_fd(struct Node**, int);
void print_clients(struct Node*);

#endif