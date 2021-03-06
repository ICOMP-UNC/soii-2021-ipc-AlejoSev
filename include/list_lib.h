#ifndef _LIST_LIB_H_
#define _LIST_LIB_H_
#define PACKET_LENGTH 128

struct Node {
    int fd;
    int address;
    int msg_i;
    long int d_time;
    char p_messages[128][64];
    struct Node* next;
};

void push_client(struct Node**, int, int);
void delete_client_by_address(struct Node**, int);
void delete_client_by_fd(struct Node**, int);
void print_clients(struct Node*);
void add_msg(struct Node**, int, char []);
int get_address_by_fd(struct Node*, int);
void set_d_time(struct Node**, int);

#endif