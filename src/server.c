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
#include "../include/list_lib.h"
#include "../include/md5.h"

#define MAX_EVENT 10
#define PACKET_LENGTH 128
#define NUM_HOSTS 5000

struct msgbuf{
    long mtype;
    char mtext[64];
};

int main(int argc, char *argv[]){
	int listen_sock, client_sock, puerto, epollfd, rdy_fds;
	char writing_buffer[PACKET_LENGTH] = "";
	char reading_buffer[PACKET_LENGTH];
	char sserver_address[5];
	int client_address, aux_address, aux_fd;
	char* token;
	char* cmd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	ssize_t bytes_readed;
	struct epoll_event event_config;
	struct epoll_event event_list[MAX_EVENT];

	// ----------------------------------------------------------------------------------------------------------

	unsigned char digest[MD5_DIGEST_LENGTH];
	char hash[(MD5_DIGEST_LENGTH * 2) + 1];

	// ----------------------------------------------------------------------------------------------------------

	struct Node* connected_clients = NULL;
	struct Node* productor1_subs = NULL;
	struct Node* productor2_subs = NULL;
	struct Node* productor3_subs = NULL;
	struct Node* aux;

	// ----------------------------------------------------------------------------------------------------------

	key_t msg_queue_key;
    struct msgbuf msgp;
    int qid;

    msg_queue_key = ftok("/home/alejo/soii-2021-ipc-AlejoSev/src/server.c", 1);

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    if((qid = msgget(msg_queue_key, 0666 | IPC_CREAT)) == -1){
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

	//-----------------------------------------------------------------------------------------------------------

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
	sprintf(sserver_address, "%d", serv_addr.sin_addr.s_addr);

	if(bind(listen_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("bind() failed.\n");
		exit(EXIT_FAILURE);
	}

	printf("---Server---\n");
	printf("Process %d - Communication by port %d\n", getpid(), ntohs(serv_addr.sin_port));

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

				event_config.events = EPOLLIN | EPOLLOUT | EPOLLET;
				event_config.data.fd = client_sock;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &event_config) == -1){
					perror("epoll_ctl() on client_sock failed.\n");
					exit(EXIT_FAILURE);
				}

				compute_md5("Checksum_Request", digest);
				for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
					sprintf(hash+j, "%02x", digest[i]);

    			hash[MD5_DIGEST_LENGTH * 2] = 0;
				bzero(writing_buffer, PACKET_LENGTH);
				sprintf(writing_buffer, "S %s %s Checksum_Request %s", sserver_address, argv[1], hash);

				if(write(client_sock, &writing_buffer, PACKET_LENGTH) == -1){
					perror("write() failed.\n");
					exit(EXIT_FAILURE);
				}
				
				bzero(writing_buffer, PACKET_LENGTH);
			}
			else{
				if(event_list[i].events & EPOLLIN){
					bytes_readed = read(event_list[i].data.fd, &reading_buffer, PACKET_LENGTH);
					
					if(bytes_readed == -1){
						perror("read() failed.\n");
						exit(EXIT_FAILURE);
					}
					else if(bytes_readed != 0){
						printf("Message received: '%s' from %d\n", reading_buffer, event_list[i].data.fd);

						token = strtok(reading_buffer, " ");

						if(strcmp(token, "H") == 0){
							token = strtok(NULL, " ");
							client_address = atoi(token);
							token = strtok(NULL, " ");
							token = strtok(NULL, " ");

							if(strcmp(token, "Checksum_Acknowledge") == 0){
								push_client(&connected_clients, event_list[i].data.fd, client_address);
								print_clients(connected_clients);
							}
						}
						else if(strcmp(token, "C") == 0){
							token = strtok(NULL, " ");
							cmd = token;
							token = strtok(NULL, " ");
							aux_address = atoi(token);

							aux = connected_clients;

							while(aux != NULL){
								if(aux->address == aux_address){
									aux_fd = aux->fd;
								}

								aux = aux->next;
							}

							if(strcmp(cmd, "add") == 0){
								token = strtok(NULL, " ");

								if(strcmp(token, "productor1") == 0){
									push_client(&productor1_subs, aux_fd, aux_address);
								}
								else if(strcmp(token, "productor2") == 0){
									push_client(&productor2_subs, aux_fd, aux_address);
								}
								else if(strcmp(token, "productor3") == 0){
									push_client(&productor3_subs, aux_fd, aux_address);
								}

								printf("Prod1 Subs:\n");
								print_clients(productor1_subs);

								printf("Prod2 Subs:\n");
								print_clients(productor2_subs);

								printf("Prod3 Subs:\n");
								print_clients(productor3_subs);
							}
							else if(strcmp(cmd, "delete") == 0){
								token = strtok(NULL, " ");

								if(strcmp(token, "productor1") == 0){
									delete_client_by_address(&productor1_subs, aux_address);
								}
								else if(strcmp(token, "productor2") == 0){
									delete_client_by_address(&productor2_subs, aux_address);
								}
								else if(strcmp(token, "productor3") == 0){
									delete_client_by_address(&productor3_subs, aux_address);
								}

								printf("Prod1 Subs:\n");
								print_clients(productor1_subs);

								printf("Prod2 Subs:\n");
								print_clients(productor2_subs);

								printf("Prod3 Subs:\n");
								print_clients(productor3_subs);
							}
							else if(strcmp(cmd, "log") == 0){
								printf("Log function\n");
							}
						}
						else{
							printf("\nProblems bro\n");
						}						
					}
					else{
						delete_client_by_fd(&connected_clients, event_list[i].data.fd);
						printf("Process %d - Socket %d disconnected\n", getpid(), event_list[i].data.fd);
						print_clients(connected_clients);
					}
				}
			}
		}

		if(msgrcv(qid, (void*)&msgp, sizeof(msgp.mtext), 0, IPC_NOWAIT) == -1){

		}
		else{
			printf("Message received: %s type: %ld\n", msgp.mtext, msgp.mtype);
			bzero(writing_buffer, PACKET_LENGTH);

			compute_md5(msgp.mtext, digest);

			for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
				sprintf(hash+j, "%02x", digest[i]);

			hash[MD5_DIGEST_LENGTH * 2] = 0;

			sprintf(writing_buffer, "S %s %s %s %s", sserver_address, argv[1], msgp.mtext, hash);

			if(msgp.mtype == 2){
				aux = productor1_subs;

				while(aux != NULL){
					if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
						perror("write() failed.\n");
						exit(EXIT_FAILURE);
					}

					aux = aux->next;
				}
			}
			else if(msgp.mtype == 3){
				aux = productor2_subs;

				while(aux != NULL){
					if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
						perror("write() failed.\n");
						exit(EXIT_FAILURE);
					}

					aux = aux->next;
				}
			}
			else if(msgp.mtype == 4){
				aux = productor3_subs;

				while(aux != NULL){
					if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
						perror("write() failed.\n");
						exit(EXIT_FAILURE);
					}

					aux = aux->next;
				}
			}

			bzero(writing_buffer, PACKET_LENGTH);
		}
	}
	return 0; 
}