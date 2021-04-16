#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PACKET_LENGTH 64

struct msgbuf{
    long mtype;
    char mtext[PACKET_LENGTH];
};

int main(){
    key_t msg_queue_key;
    struct msgbuf msgp;
    int qid;

    msgp.mtype = 2;
    strcpy(msgp.mtext, "productor1 up");

    msg_queue_key = ftok("/home/alejo/soii-2021-ipc-AlejoSev/src/server.c", 1);

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Key: %d\n", msg_queue_key);

    if(qid = msgget(msg_queue_key, 0666) == -1){
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        if(msgsnd(qid, (void*)&msgp, sizeof(msgp.mtext), IPC_NOWAIT) == -1){
            perror("msgsnd() failed.");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }

    return 0;
}