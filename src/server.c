#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_EVENT 10
#define PACKET_LENGTH 64
#define NUM_HOSTS 5000

void command_line_interface();

int main(int argc, char *argv[]){
	int listen_sock, client_sock, puerto, epollfd, rdy_fds;
	char checksum_buffer[PACKET_LENGTH] = "Cheksum Message";
	char reading_buffer[PACKET_LENGTH];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
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

	listen(listen_sock, NUM_HOSTS);

	clilen = sizeof(cli_addr);

	epollfd = epoll_create(NUM_HOSTS);

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
			else if(event_list[i].events & EPOLLIN){
				bytes_readed = read(event_list[i].data.fd, &reading_buffer, PACKET_LENGTH);
				
				if(bytes_readed == -1){
					perror("read() failed.\n");
					exit(EXIT_FAILURE);
				}
				else if(bytes_readed != 0){
					printf("Received message: '%s' from %d.\n", reading_buffer, event_list[i].data.fd);
				}
				else{
					printf("Proceso: %d - socket desconectado: %d\n", getpid(), event_list[i].data.fd);
				}
			}
		}
	}
	return 0; 
}