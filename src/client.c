#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PACKET_LENGTH 128

int main(int argc, char *argv[]){
	int sockfd, puerto;
	ssize_t bytes_readed;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char reading_buffer[PACKET_LENGTH];
	char writing_buffer[PACKET_LENGTH];
	char* token;

	if(argc < 3){
		fprintf(stderr, "Uso %s hostname port client_address\n", argv[0]);
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

	// char puertovich[2];
	// sprintf(puertovich, "%d", serv_addr.sin_port);
	// printf("Puertovich: %s\n", puertovich);

	while(1){
		bzero(reading_buffer, PACKET_LENGTH);
		bzero(writing_buffer, PACKET_LENGTH);

		bytes_readed = read(sockfd, &reading_buffer, PACKET_LENGTH);

		if(bytes_readed == -1){
			perror("read() failed.\n");
			exit(EXIT_FAILURE);
		}

		printf("(%s)Message received: '%s'\n", argv[3], reading_buffer);

		token = strtok(reading_buffer, " ");
		token = strtok(NULL, " ");
		token = strtok(NULL, " ");
		token = strtok(NULL, " ");

		if(strncmp(token,"Checksum_Request", 16) == 0){
			strcat(writing_buffer, "H");
			strcat(writing_buffer, " ");
			strcat(writing_buffer, argv[3]);
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
		else{
			strcat(writing_buffer, "H");
			strcat(writing_buffer, " ");
			strcat(writing_buffer, argv[3]);
			strcat(writing_buffer, " ");
			strcat(writing_buffer, argv[2]);
			strcat(writing_buffer, " ");
			strcat(writing_buffer, "Message_Acknowledge");
			strcat(writing_buffer, " ");
			strcat(writing_buffer, "Checksum_Hash");

			if(write(sockfd, &writing_buffer, PACKET_LENGTH) == -1){
				perror("write() failed.\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
} 