#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define PACKET_LENGTH 64

int main(int argc, char *argv[]){
	int sockfd, puerto;
	ssize_t bytes_readed;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char reading_buffer[PACKET_LENGTH];
	char writing_buffer[PACKET_LENGTH] = "Acknowledge";

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
		bytes_readed = read(sockfd, &reading_buffer, PACKET_LENGTH);

		if(bytes_readed == -1){
			perror("read() failed.\n");
			exit(EXIT_FAILURE);
		}

		printf("Message received: %s.\n", reading_buffer);

		if(write(sockfd, &writing_buffer, PACKET_LENGTH) == -1){
			perror("write() failed.\n");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
} 