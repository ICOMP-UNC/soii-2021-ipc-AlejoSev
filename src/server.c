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
#include <time.h>
#include <signal.h>
#include <zip.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "../include/list_lib.h"
#include "../include/md5.h"

#define MAX_EVENT 10
#define PACKET_LENGTH 128
#define NUM_HOSTS 5000

struct msgbuf{
    long mtype;
    char mtext[64];
};

int get_address_by_fd(struct Node* n, int fd);
int check_match(struct Node* n, int address);
void create_log_zip();
void update_time();
void close_server();

FILE* fp;
time_t t;
struct tm *c_time;

int main(int argc, char *argv[]){
	int listen_sock, client_sock, puerto, epollfd, rdy_fds, client_address, aux_address, aux_fd, qid;
	char hash[(MD5_DIGEST_LENGTH * 2) + 1], sserver_address[5], reading_buffer[PACKET_LENGTH], writing_buffer[PACKET_LENGTH] = "";
	char* token, *cmd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	ssize_t bytes_readed;
	struct epoll_event event_config, event_list[MAX_EVENT];
	unsigned char digest[MD5_DIGEST_LENGTH];
	struct Node* connected_clients = NULL;
	struct Node* disconnected_clients = NULL;
	struct Node* productor1_subs = NULL;
	struct Node* productor2_subs = NULL;
	struct Node* productor3_subs = NULL;
	struct Node* aux;
	key_t msg_queue_key;
    struct msgbuf msgp;

    msg_queue_key = ftok("/home/alejo/sisops/repos/soii-2021-ipc-AlejoSev/src/server.c", 1);			//Generamos clave

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    if((qid = msgget(msg_queue_key, 0666 | IPC_CREAT)) == -1){							//Obtenemos queue
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

	if(argc < 2){
		fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
		exit(1);
	}

	signal(SIGINT, close_server);

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);										//Creamos socket

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

	listen(listen_sock, NUM_HOSTS);														//Lo ponemos a escuchar

	clilen = sizeof(cli_addr);

	epollfd = epoll_create(NUM_HOSTS);													//Creamos epoll

	if(epollfd == -1){
		perror("epoll_create() failed.\n");
		exit(EXIT_FAILURE);
	}

	event_config.events = EPOLLIN;
	event_config.data.fd = listen_sock;

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &event_config) == -1){			//Agregamos listen_sock al epoll
		perror("epol_ctl() on listen_sock failed.\n");
		exit(EXIT_FAILURE);
	}

	fp = fopen("log.txt", "a");

	if(fp == NULL){
		perror("fopen() failed.\n");
		exit(EXIT_FAILURE);
	}

	update_time();
	printf("[%02d:%02d:%02d] --- Server Up ---\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec);
	fprintf(fp, "[%02d:%02d:%02d] --- Server Up ---\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec);

	while(1){
		rdy_fds = epoll_wait(epollfd, event_list, MAX_EVENT, 500);						//Esperamos eventos

		if(rdy_fds == -1){
			perror("epoll_wait() failed.\n");
			exit(EXIT_FAILURE);
		}

		for(int i = 0; i<rdy_fds; i++){
			if(event_list[i].data.fd == listen_sock){
				client_sock = accept(listen_sock, (struct sockaddr*)&cli_addr, &clilen);	//Aceptamos nuevo cliente

				if(client_sock == -1){
					perror("accept() failed.\n");
					exit(EXIT_FAILURE);
				}

				event_config.events = EPOLLIN | EPOLLOUT | EPOLLET;
				event_config.data.fd = client_sock;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &event_config) == -1){	//Lo agregamos al epoll
					perror("epoll_ctl() on client_sock failed.\n");
					exit(EXIT_FAILURE);
				}

				compute_md5("Checksum_Request", digest);
				for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
					sprintf(hash+j, "%02x", digest[i]);

    			hash[MD5_DIGEST_LENGTH * 2] = 0;

				bzero(writing_buffer, PACKET_LENGTH);
				sprintf(writing_buffer, "S %s %s Checksum_Request %s", sserver_address, argv[1], hash);

				if(write(client_sock, &writing_buffer, PACKET_LENGTH) == -1){				//Enviamos el primer mensaje al cliente
					perror("write() failed.\n");
					exit(EXIT_FAILURE);
				}
				
				bzero(writing_buffer, PACKET_LENGTH);
			}
			else{
				if(event_list[i].events & EPOLLIN){
					bytes_readed = read(event_list[i].data.fd, &reading_buffer, PACKET_LENGTH);		//Leemos mensaje entrante
					
					if(bytes_readed == -1){
						perror("read() failed.\n");
						exit(EXIT_FAILURE);
					}
					else if(bytes_readed != 0){
						token = strtok(reading_buffer, " ");

						if(strcmp(token, "H") == 0){				//En caso de ser cliente
							token = strtok(NULL, " ");
							client_address = atoi(token);
							token = strtok(NULL, " ");
							token = strtok(NULL, " ");

							if((strcmp(token, "Checksum_Acknowledge") == 0) && (check_match(connected_clients, client_address) == 0)){	//En caso de no estar en la lista de conectados y que sea checksum_ack
								if(check_match(disconnected_clients, client_address)){													//Si estaba en la lista de desconectados
									aux = disconnected_clients;

									while(aux != NULL){
										if(aux->address == client_address){
											delete_client_by_address(&disconnected_clients, client_address);
											
											if(time(NULL)-aux->d_time < 5){										//Si el tiempo de desconexion es menor a 5 segs
												for(int j = 0; j<aux->msg_i; j++){								//Le mando todos los mensajes guardados para el
													compute_md5(aux->p_messages[i], digest);

													for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
														sprintf(hash+j, "%02x", digest[i]);

													hash[MD5_DIGEST_LENGTH * 2] = 0;
													sprintf(writing_buffer, "S %s %s %s %s", sserver_address, argv[1], aux->p_messages[j], hash);

													if(write(event_list[i].data.fd, writing_buffer, PACKET_LENGTH) == -1){
														perror("write() failed.\n");
														exit(EXIT_FAILURE);
													}
												}

												if(check_match(productor1_subs, client_address)){							//Renuevo file descriptor en todas las listas
													delete_client_by_address(&productor1_subs, client_address);
													push_client(&productor1_subs, event_list[i].data.fd, client_address);
												}
												if(check_match(productor2_subs, client_address)){
													delete_client_by_address(&productor2_subs, client_address);
													push_client(&productor2_subs, event_list[i].data.fd, client_address);
												}
												if(check_match(productor3_subs, client_address)){
													delete_client_by_address(&productor3_subs, client_address);
													push_client(&productor3_subs, event_list[i].data.fd, client_address);
												}
											}
											else{																			//Si fueron mas de 5 segs lo saco de las listas
												if(check_match(productor1_subs, client_address)){
													delete_client_by_address(&productor1_subs, client_address);
												}
												if(check_match(productor2_subs, client_address)){
													delete_client_by_address(&productor2_subs, client_address);
												}
												if(check_match(productor3_subs, client_address)){
													delete_client_by_address(&productor3_subs, client_address);
												}

												close(aux->fd);
											}
										}

										aux = aux->next;
									}
								}

								push_client(&connected_clients, event_list[i].data.fd, client_address);					//Lo agrego a la lista de conectados
								update_time();
								fprintf(fp, "[%02d:%02d:%02d] Client %d Connected\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, client_address);
							}
						}
						else if(strcmp(token, "C") == 0){				// En caso de ser la CLI
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

								if((strcmp(token, "productor1") == 0) && !check_match(productor1_subs, aux_address) && check_match(connected_clients, aux_address)){
									push_client(&productor1_subs, aux_fd, aux_address);
									update_time(); 
									fprintf(fp, "[%02d:%02d:%02d] Client %d Subscribed to Productor1\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
								else if((strcmp(token, "productor2") == 0) && !check_match(productor2_subs, aux_address) && check_match(connected_clients, aux_address)){
									push_client(&productor2_subs, aux_fd, aux_address);
									update_time();
									fprintf(fp, "[%02d:%02d:%02d] Client %d Subscribed to Productor2\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
								else if((strcmp(token, "productor3") == 0) && !check_match(productor3_subs, aux_address) && check_match(connected_clients, aux_address)){
									push_client(&productor3_subs, aux_fd, aux_address);
									update_time();
									fprintf(fp, "[%02d:%02d:%02d] Client %d Subscribed to Productor3\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
							}
							else if(strcmp(cmd, "delete") == 0){
								token = strtok(NULL, " ");

								if((strcmp(token, "productor1") == 0) && (check_match(productor1_subs, aux_address) == 1)){
									delete_client_by_address(&productor1_subs, aux_address);
									update_time();
									fprintf(fp, "[%02d:%02d:%02d] Client %d Unsubscribed from Productor1\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
								else if((strcmp(token, "productor2") == 0) && (check_match(productor2_subs, aux_address) == 1)){
									delete_client_by_address(&productor2_subs, aux_address);
									update_time();
									fprintf(fp, "[%02d:%02d:%02d] Client %d Unsubscribed from Productor2\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
								else if((strcmp(token, "productor3") == 0) && (check_match(productor3_subs, aux_address) == 1)){
									delete_client_by_address(&productor3_subs, aux_address);
									update_time();
									fprintf(fp, "[%02d:%02d:%02d] Client %d Unsubscribed from Productor3\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
								}
							}
							else if(strcmp(cmd, "log") == 0){
								// client_address = strtok(NULL, " ");
								ssize_t s;

								create_log_zip();

								struct stat zip_stat;

								int zip_fd = open("log.zip", O_RDONLY);
								if(zip_fd == -1){
									perror("Zip fopen() failed.\n");
									exit(EXIT_FAILURE);
								}

								if(fstat((zip_fd), &zip_stat) == -1){
									perror("fstat() failed.\n");
									exit(EXIT_FAILURE);
								}

								bzero(writing_buffer, PACKET_LENGTH);
								sprintf(writing_buffer, "L %ld", zip_stat.st_size);

								if(write(aux_fd, writing_buffer, PACKET_LENGTH) == -1){
									perror("write() failed.\n");
									exit(EXIT_FAILURE);
								}

								bzero(writing_buffer, PACKET_LENGTH);

								printf("File size: %ld bytes.\n", zip_stat.st_size);

								s = sendfile(aux_fd, zip_fd, NULL, (size_t)zip_stat.st_size);
								if(s<0){
									perror("senfile() filed.\n");
									exit(EXIT_FAILURE);
								}
								
								close(zip_fd);
							}
						}
						else{
							printf("\nProblems bro\n");
						}						
					}
					else{																					//En caso de que se desconecte un cliente
						aux_address = get_address_by_fd(connected_clients, event_list[i].data.fd);
						push_client(&disconnected_clients, -1, aux_address);
						set_d_time(&disconnected_clients, aux_address);
						delete_client_by_fd(&connected_clients, event_list[i].data.fd);
						update_time();
						fprintf(fp, "[%02d:%02d:%02d] Client %d Disconnected\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, aux_address);
					}
				}
			}
		}

		if(msgrcv(qid, (void*)&msgp, sizeof(msgp.mtext), 0, IPC_NOWAIT) == -1){		//Leo mensaje de la Queue

		}
		else{
			printf("Message received: %s type: %ld\n", msgp.mtext, msgp.mtype);
			bzero(writing_buffer, PACKET_LENGTH);

			compute_md5(msgp.mtext, digest);

			for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
				sprintf(hash+j, "%02x", digest[i]);

			hash[MD5_DIGEST_LENGTH * 2] = 0;

			sprintf(writing_buffer, "S %s %s %s %s", sserver_address, argv[1], msgp.mtext, hash);

			if(msgp.mtype == 2){													//Si es de productor1
				aux = productor1_subs;

				while(aux != NULL){
					aux_address = get_address_by_fd(productor1_subs, aux->fd);

					if(!check_match(disconnected_clients, aux_address)){			//Si esta conectado el cliente
						if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
							perror("write() failed.\n");
							exit(EXIT_FAILURE);
						}
						
						update_time();
						fprintf(fp, "[%02d:%02d:%02d] '%s' Sent from Productor1 to Client %d\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, msgp.mtext, aux_address);
					}
					else{															//Si no esta conectado el cliente
						add_msg(&disconnected_clients, aux_address, msgp.mtext);
					}

					aux = aux->next;
				}
			}
			else if(msgp.mtype == 3){												//Si es de productor2
				aux = productor2_subs;

				while(aux != NULL){
					aux_address = get_address_by_fd(productor2_subs, aux->fd);

					if(!check_match(disconnected_clients, aux_address)){			//Si esta conectado el cliente
						if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
							perror("write() failed.\n");
							exit(EXIT_FAILURE);
						}
						
						update_time();
						fprintf(fp, "[%02d:%02d:%02d] '%s' Sent from Productor2 to Client %d\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, msgp.mtext, aux_address);
					}
					else{															//Si no esta conectado el cliente
						add_msg(&disconnected_clients, aux_address, msgp.mtext);
					}

					aux = aux->next;
				}
			}
			else if(msgp.mtype == 4){												//Si es de productor3
				aux = productor3_subs;

				while(aux != NULL){
					aux_address = get_address_by_fd(productor3_subs, aux->fd);

					if(!check_match(disconnected_clients, aux_address)){			//Si esta conectado el cliente
						if(write(aux->fd, writing_buffer, PACKET_LENGTH) == -1){
							perror("write() failed.\n");
							exit(EXIT_FAILURE);
						}
						
						update_time();
						fprintf(fp, "[%02d:%02d:%02d] '%s' Sent from Productor3 to Client %d\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec, msgp.mtext, aux_address);
					}
					else{															//Si no esta conectado el cliente
						add_msg(&disconnected_clients, aux_address, msgp.mtext);
					}

					aux = aux->next;
				}
			}

			bzero(writing_buffer, PACKET_LENGTH);
		}
	}
	return 0; 
}

int check_match(struct Node* n, int address){
	struct Node* aux = n;

	while(aux != NULL){
		if(aux->address == address){
			return 1;
		}

		aux = aux->next;
	}

	return 0;
}

void create_log_zip(){
	fclose(fp);
	printf("Cerre el log\n");

	fp = fopen("log.txt", "r");
	char* buffer = NULL;
	long int size = 0;
	
	rewind(fp);								//Calculo tamaño del archivo
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	rewind(fp);
	buffer = malloc(((long unsigned int)size + 1) * sizeof(buffer));
	fread(buffer, (long unsigned int)size, 1, fp);

	buffer[size] = '\0';
	printf("%s", buffer);

	int err = 0;
	zip_t* z = zip_open("log.zip", ZIP_CREATE, &err);
	if(z == NULL){
		perror("Error al clear el archivo zip\n");
		exit(EXIT_FAILURE);
	}
	zip_source_t* zs;
	zs = zip_source_buffer(z, buffer, strlen(buffer), 0);
	zip_file_add(z, "log.txt", zs, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);

	zip_close(z); 

	fclose(fp);

	fp = fopen("log.txt", "a");
	free(buffer);
}

void update_time(){
	t = time(NULL);
	c_time = localtime(&t);
}

void close_server(){
	t = time(NULL);
	c_time = localtime(&t);
	printf("[%02d:%02d:%02d] --- Server Down ---\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec);
	fprintf(fp, "[%02d:%02d:%02d] --- Server Down ---\n", c_time->tm_hour, c_time->tm_min, c_time->tm_sec);
	fclose(fp);
	exit(EXIT_SUCCESS);
}