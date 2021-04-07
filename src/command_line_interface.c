#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(){
    char user_input[64];
    char** parsed_input;
    char* token;
    int word_counter;

    while(1){
        printf("> ");
        fgets(user_input, 63, stdin);
        strtok(user_input, "\n");

        parsed_input = NULL;
        word_counter = 0;

        token = strtok(user_input, " ");
        while(token != NULL){
            parsed_input = (char**)reallocarray(parsed_input, word_counter+1, sizeof(char*));
            parsed_input[word_counter] = (char*)calloc(strlen(token), sizeof(char*));
            strncpy(parsed_input[word_counter], token, strlen(token));

            word_counter++;
            token = strtok(NULL, " ");
        }

        if(!strcmp(parsed_input[0], "add")){
            printf("Adding %s to %s\n", parsed_input[1], parsed_input[2]);
        }
        else if(!strcmp(parsed_input[0], "delete")){
            printf("calling delete function...\n");
        }
        else if(!strcmp(parsed_input[0], "log")){
            printf("calling log function...\n");
        }
        else{
            printf("command not found. try again.\n");
        }
    }

    return 0;
}