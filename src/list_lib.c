#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/list_lib.h"

/* Given a reference (pointer to pointer) to the head of a
   list and an int, inserts a new node on the front of the
   list. */
void push_client(struct Node** head_ref, int new_fd, int new_address){
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    new_node->fd = new_fd;
    new_node->address = new_address;
    new_node->msg_i = 0;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}
 
/* Given a reference (pointer to pointer) to the head of a
   list and a key, deletes the first occurrence of key in
   linked list */
void delete_client_by_address(struct Node** head_ref, int key){
    // Store head node
    struct Node *temp = *head_ref, *prev;
 
    // If head node itself holds the key to be deleted
    if(temp != NULL && temp->address == key){
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while(temp != NULL && temp->address != key){
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

void delete_client_by_fd(struct Node** head_ref, int key){
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
void print_clients(struct Node* node){
    while(node != NULL){
        printf("File Descriptor: %d - Address: %d\n", node->fd, node->address);
        node = node->next;
    }
}

void add_msg(struct Node** head_ref, int address, char s[]){
    struct Node* node = *head_ref;

    while(node != NULL){
        if(node->address == address){
            strcpy(node->p_messages[node->msg_i], s);
            printf("p_message: %s\n", node->p_messages[node->msg_i]);
            node->msg_i++;
        }

        node = node->next;
    }
}

int get_address_by_fd(struct Node* n, int fd){
	struct Node* aux = n;
	int address;

	while(aux != NULL){
		if(aux->fd == fd){
			address = aux->address;
		}

		aux = aux->next;
	}

	return address;
}

void set_d_time(struct Node** head_ref, int address){
    struct Node* node = *head_ref;

    while(node != NULL){
        if(node->address == address){
            node->d_time = time(NULL);
        }

        node = node->next;
    }
}