#include <stdio.h>
#include <stdlib.h>
#include "../include/list_lib.h"

/* Given a reference (pointer to pointer) to the head of a
   list and an int, inserts a new node on the front of the
   list. */
void push_client(struct Node** head_ref, int new_fd, int new_address){
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    new_node->fd = new_fd;
    new_node->address = new_address;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}
 
/* Given a reference (pointer to pointer) to the head of a
   list and a key, deletes the first occurrence of key in
   linked list */
void delete_client(struct Node** head_ref, int key){
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if(temp != NULL && temp->fd == key){
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while(temp != NULL && temp->fd != key){
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if(temp == NULL)
        return;
 
    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
}
 
// This function prints contents of linked list starting
// from the given node
void print_clients(struct Node* node)
{
    while (node != NULL) {
        printf("File Descriptor: %d - Address: %d\n", node->fd, node->address);
        node = node->next;
    }
}