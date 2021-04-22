#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PACKET_LENGTH 128

struct msgbuf{
    long mtype;
    char mtext[PACKET_LENGTH];
};

int main(){
    key_t msg_queue_key;
    struct msgbuf msgp;
    int qid, fd;
    char buffer[64], to_send[64];
    float load, n_cpu_load;
    int nprocs = get_nprocs();
    char* token;

    msgp.mtype = 4;
    msg_queue_key = ftok("/home/alejo/soii-2021-ipc-AlejoSev/src/server.c", 1);

    if(msg_queue_key == -1){
        perror("ftok() failed.\n");
        exit(EXIT_FAILURE);
    }

    if((qid = msgget(msg_queue_key, 0666)) == -1){
        perror("msgget() failed.\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        fd = open("/proc/loadavg", O_RDONLY);

        if(fd == -1) {
            perror("open() failed.\n");
            exit(EXIT_FAILURE); 
        }

        read(fd, buffer, sizeof(buffer)-1);
        
        token = strtok(buffer, " ");
        load = (float)atof(token);
        n_cpu_load = load*100/(float)nprocs;

        sprintf(to_send, "%f", n_cpu_load);
        strcpy(msgp.mtext, to_send);

        printf("%s\n", msgp.mtext);

        if(msgsnd(qid, (void*)&msgp, sizeof(msgp.mtext), IPC_NOWAIT) == -1){
            perror("msgsnd() failed.");
            exit(EXIT_FAILURE);
        }

        sleep(1);
        close(fd);
    }

    return 0;
}