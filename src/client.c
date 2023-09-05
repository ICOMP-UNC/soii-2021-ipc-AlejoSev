#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "../include/md5.h"

#define PACKET_LENGTH 128

int main(int argc, char *argv[]){
	int sockfd, puerto;
	long int zip_size;
	ssize_t bytes_readed;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char reading_buffer[PACKET_LENGTH];
	char writing_buffer[PACKET_LENGTH];
	char* token, *endptr;

	unsigned char digest[MD5_DIGEST_LENGTH];
	char hash[(MD5_DIGEST_LENGTH * 2) + 1];

	if(argc < 3){
		fprintf(stderr, "Uso %s hostname port client_address\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	puerto = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);														//Creo socket

	server = gethostbyname(argv[1]);

	memset((char*)&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);
	serv_addr.sin_port = htons((uint16_t)puerto);

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){						//Conecto socket contra server
		perror("connect() failed.\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		bzero(reading_buffer, PACKET_LENGTH);
		bzero(writing_buffer, PACKET_LENGTH);

		bytes_readed = read(sockfd, &reading_buffer, PACKET_LENGTH);								//Quedo esperando mensaje

		if(bytes_readed == -1){
			perror("read() failed.\n");
			exit(EXIT_FAILURE);
		}

		printf("(%s)Message received: '%s'\n", argv[3], reading_buffer);

		token = strtok(reading_buffer, " ");

		if(strncmp(token, "L", 1) == 0){
			ssize_t read_bytes;

			FILE *zipfile = fopen("client_log.zip", "wb");
			if (zipfile == NULL){
				perror("fopen() failed.\n");
			}

			token = strtok(NULL, " ");
			zip_size = strtol(token, &endptr, 10);
			printf("Zip size: %ld\n", zip_size);

			char* zipdata = calloc((size_t)zip_size, 1);

			read_bytes = read(sockfd, zipdata, (size_t)zip_size);
			if(read_bytes < 0){
				perror("read() failed.\n");
			}

			printf("Bytes readed: %ld\n", read_bytes);

			fwrite(zipdata, 1, (size_t)zip_size, zipfile);
				
			int close = fclose(zipfile);
			if(close != 0){
				perror("fclose() failed.\n");
			}
		}
		else{
			token = strtok(NULL, " ");
			token = strtok(NULL, " ");
			token = strtok(NULL, " ");

			if(strncmp(token,"Checksum_Request", 16) == 0){												//Si es checksum_request envio checksum_acknowledge
				compute_md5(token, digest);

				for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
					sprintf(hash+j, "%02x", digest[i]);

				hash[MD5_DIGEST_LENGTH * 2] = 0;
				token = strtok(NULL, " ");

				if(strncmp(token, hash, MD5_DIGEST_LENGTH*2) == 0){
					compute_md5("Checksum_Acknowledge", digest);

					for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
						sprintf(hash+j, "%02x", digest[i]);

					hash[MD5_DIGEST_LENGTH * 2] = 0;

					sprintf(writing_buffer, "H %s %s Checksum_Acknowledge %s", argv[3], argv[2], hash);

					if(write(sockfd, &writing_buffer, PACKET_LENGTH) == -1){
						perror("write() failed.\n");
						exit(EXIT_FAILURE);
					}

					bzero(writing_buffer, PACKET_LENGTH);
				}
			}
			else{																						//En caso de ser otro mensaje simplemente devuelvo message_acknowledge
				compute_md5(token, digest);

				for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
					sprintf(hash+j, "%02x", digest[i]);

				hash[MD5_DIGEST_LENGTH * 2] = 0;
				token = strtok(NULL, " ");

				if(strncmp(token, hash, MD5_DIGEST_LENGTH*2) == 0){
					compute_md5("Message_Acknowledge", digest);

					for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
						sprintf(hash+j, "%02x", digest[i]);

					hash[MD5_DIGEST_LENGTH * 2] = 0;
					bzero(writing_buffer, PACKET_LENGTH);
					sprintf(writing_buffer, "H %s %s Message_Acknowledge %s", argv[3], argv[2], hash);

					if(write(sockfd, &writing_buffer, PACKET_LENGTH) == -1){
						perror("write() failed.\n");
						exit(EXIT_FAILURE);
					}

					bzero(writing_buffer, PACKET_LENGTH);
				}
			}
		}
	}

	return 0;
} 