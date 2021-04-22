#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PACKET_LENGTH 64

struct msgbuf{
    long mtype;
    char mtext[PACKET_LENGTH];
};

int main(){
    key_t msg_queue_key;
    struct msgbuf msgp;
    int qid;
    FILE* fp;
    char buffer[100];
    char* token;

    msgp.mtype = 3;
    strcpy(msgp.mtext, "productor2_up");

    msg_queue_key = ftok("/home/alejo/soii-2021-ipc-AlejoSev/src/server.c", 1);

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Key: %d\n", msg_queue_key);

    if((qid = msgget(msg_queue_key, 0666)) == -1){
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        fp = fopen("/proc/meminfo", "r");

        if(fp == NULL){
            printf("fopen() failed.\n");
            exit(EXIT_FAILURE);
        }

        while(fgets(buffer,100,fp)){
            if(strncmp(buffer, "MemFree", 7) == 0){
                token = strtok(buffer,":");
                token = strtok(NULL,":");
                token = strtok(token," ");
                sprintf(msgp.mtext, "%s", token);
            }
        }

        if(msgsnd(qid, (void*)&msgp, sizeof(msgp.mtext), IPC_NOWAIT) == -1){
            perror("msgsnd() failed.");
            exit(EXIT_FAILURE);
        }
        printf("%s\n", msgp.mtext);

        sleep(1);
        fclose(fp);
    }

    return 0;
}