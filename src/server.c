#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>

#define TAM 256
#define MAX_EVENT 10
#define PACKET_LENGTH 16

int main(int argc, char *argv[]){
	int listen_sock, client_sock, puerto, epollfd, rdy_fds;
	// int pid;
	// char buffer[TAM];
	char checksum_buffer[PACKET_LENGTH] = "Cheksum Message";
	char reading_buffer[PACKET_LENGTH];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	// ssize_t n;
	ssize_t bytes_readed;
	struct epoll_event event_config;
	struct epoll_event event_list[MAX_EVENT];

	if(argc < 2){
		fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
		exit(1);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	memset((char*)&serv_addr, 0, sizeof(serv_addr));

	puerto = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons((uint16_t)puerto);

	if(bind(listen_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("bind() failed.\n");
		exit(EXIT_FAILURE);
	}

	printf("Proceso: %d - socket disponible: %d\n", getpid(), ntohs(serv_addr.sin_port));

	listen(listen_sock, 5);

	clilen = sizeof(cli_addr);

	epollfd = epoll_create(5000);

	if(epollfd == -1){
		perror("epoll_create() failed.\n");
		exit(EXIT_FAILURE);
	}

	event_config.events = EPOLLIN;
	event_config.data.fd = listen_sock;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &event_config) == -1){
		perror("epol_ctl() on listen_sock failed.\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		rdy_fds = epoll_wait(epollfd, event_list, MAX_EVENT, 500);

		if(rdy_fds == -1){
			perror("epoll_wait() failed.\n");
			exit(EXIT_FAILURE);
		}

		for(int i = 0; i<rdy_fds; i++){
			if(event_list[i].data.fd == listen_sock){
				client_sock = accept(listen_sock, (struct sockaddr*)&cli_addr, &clilen);

				if(client_sock == -1){
					perror("accept() failed.\n");
					exit(EXIT_FAILURE);
				}

				event_config.events = EPOLLIN | EPOLLET;
				event_config.data.fd = client_sock;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &event_config) == -1){
					perror("epoll_ctl() on client_sock failed.\n");
					exit(EXIT_FAILURE);
				}

				if(write(client_sock, &checksum_buffer, PACKET_LENGTH) == -1){
					perror("write() failed.\n");
					exit(EXIT_FAILURE);
				}
			}
			else{
				bytes_readed = read(event_list[i].data.fd, &reading_buffer, PACKET_LENGTH);
				
				if(bytes_readed == -1){
					perror("read() failed.\n");
					exit(EXIT_FAILURE);
				}

				printf("Received message: %s.\n", reading_buffer);
			}
		}

		// newsockfd = accept(listen_sock, (struct sockaddr*)&cli_addr, &clilen);

		// pid = fork(); 

		// if(pid == 0){  // Proceso hijo
		//     close(listen_sock);
		    
		//     while(1){
		// 		memset(buffer, 0, TAM);
				
		// 		n = read(newsockfd, buffer, TAM-1);

		// 		if(n < 0){
		// 			perror("lectura de socket");
		// 			exit(1);
		// 		}
				
		// 		printf("PROCESO %d.", getpid());
		// 		printf("Recibí: %s", buffer);
				
		// 		n = write(newsockfd, "Obtuve su mensaje", 18);

		// 		if(n < 0){
		// 			perror("escritura en socket");
		// 			exit(1);
		// 		}
		// 		// Verificación de si hay que terminar
		// 		buffer[strlen(buffer)-1] = '\0';
		// 		if(!strcmp("fin", buffer)){
		// 			printf("PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid());
		// 			exit(0);
		// 		}
		// 	}
		// }
		// else{
		// 	printf("SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid);
		// 	close(newsockfd);
		// }
	}
	return 0; 
}