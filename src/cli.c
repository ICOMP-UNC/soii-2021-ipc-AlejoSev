#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PACKET_LENGTH 128

int main(int argc, char *argv[]){
    char user_input[64];
    char to_send[PACKET_LENGTH];
    char** parsed_input;
    char* token;
    int word_counter;
    int ready_to_send = 0;
    int first_communication = 1;

	int sockfd, puerto;
	ssize_t bytes_readed;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char reading_buffer[PACKET_LENGTH];
	char writing_buffer[PACKET_LENGTH];

	if(argc < 3){
		fprintf(stderr, "Uso %s host puerto\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	puerto = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(argv[1]);

	memset((char*)&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);
	serv_addr.sin_port = htons((uint16_t)puerto);

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("connect() failed.\n");
		exit(EXIT_FAILURE);
	}

	while(1){
        bzero(reading_buffer, PACKET_LENGTH);
		bzero(writing_buffer, PACKET_LENGTH);

        if(first_communication){
            bytes_readed = read(sockfd, &reading_buffer, PACKET_LENGTH);

            if(bytes_readed == -1){
                perror("read() failed.\n");
                exit(EXIT_FAILURE);
            }

            printf("Message received: %s.\n", reading_buffer);

            token = strtok(reading_buffer, " ");
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");

            if(strncmp(token,"Checksum_Request", 16) == 0){
                strcat(writing_buffer, "C");
                strcat(writing_buffer, " ");
                strcat(writing_buffer, "Connect");
                strcat(writing_buffer, " ");
                strcat(writing_buffer, argv[2]);
                strcat(writing_buffer, " ");
                strcat(writing_buffer, "Checksum_Acknowledge");
                strcat(writing_buffer, " ");
                strcat(writing_buffer, "Checksum_Hash");

                if(write(sockfd, &writing_buffer, PACKET_LENGTH) == -1){
                    perror("write() failed.\n");
                    exit(EXIT_FAILURE);
                }
            }

            first_communication = 0;
        }

        while(!ready_to_send){
            printf("> ");
            fgets(user_input, 63, stdin);
            strtok(user_input, "\n");            

            parsed_input = NULL;
            word_counter = 0;

            token = strtok(user_input, " ");
            while(token != NULL){
                parsed_input = (char**)reallocarray(parsed_input, (size_t)word_counter+1, sizeof(char*));
                parsed_input[word_counter] = (char*)calloc(strlen(token), sizeof(char*));
                strncpy(parsed_input[word_counter], token, strlen(token));

                word_counter++;
                token = strtok(NULL, " ");
            }

            if(!strcmp(parsed_input[0], "add")){
                printf("Adding %s to %s\n", parsed_input[1], parsed_input[2]);
                ready_to_send = 1;
            }
            else if(!strcmp(parsed_input[0], "delete")){
                printf("calling delete function...\n");
                ready_to_send = 1;
            }
            else if(!strcmp(parsed_input[0], "log")){
                printf("calling log function...\n");
                ready_to_send = 1;
            }
            else{
                printf("Command not found. Try again.\nCommand List:\n    add <client> <productor>\n    add <client> <productor>\n    log <client>\n");
            }
        }

		bzero(to_send, PACKET_LENGTH);

        strcat(to_send, "C");
        strcat(to_send, " ");
        strcat(to_send, parsed_input[0]);
        strcat(to_send, " ");
        strcat(to_send, parsed_input[1]);
        strcat(to_send, " ");
        strcat(to_send, parsed_input[2]);

        if(write(sockfd, &to_send, PACKET_LENGTH) == -1){
            perror("write() failed.\n");
            exit(EXIT_FAILURE);
        }

        ready_to_send = 0;
	}
	return 0;
} 