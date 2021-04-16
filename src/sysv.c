#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


#define PACKET_LENGTH 64

struct msgbuf{
    long mtype;
    char mtext[PACKET_LENGTH];
};

int main(){
    key_t msg_queue_key;
    struct msqid_ds msgq_config;
    struct msgbuf msgp;
    int qid;

    msg_queue_key = ftok("/home/alejo/soii-2021-ipc-AlejoSev/src/server.c", 1);

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Key: %d\n", msg_queue_key);

    if(qid = msgget(msg_queue_key, 0666 | IPC_CREAT) == -1){
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

    // if(msgctl(msg_queue_key, IPC_STAT, &msgq_config)){
    //     perror("msgctl() failed.\n");
    //     exit(EXIT_FAILURE);
    // }

    while(1){
        if(msgrcv(qid, (void*)&msgp, sizeof(msgp.mtext), 2, 0) == -1){
            perror("msgrcv() failed.");
            exit(EXIT_FAILURE);
        }

        printf("Message received: %s type: %ld\n", msgp.mtext, msgp.mtype);
    }

    return 0;
}