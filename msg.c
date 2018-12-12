#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

struct msgbuf 
{
    long mtype;
    char mtext[1024];
};

int main()
{
    int id;
    if((id = msgget(17, IPC_CREAT | 0600)) == -1)
    {
        perror("tworzenie kolejki komunikatow");
        exit(1);
    }

    if(fork() == 0)
    {
        struct msgbuf x;
        x.mtype = 10;
        strcpy(x.mtext, "Hello1\n");

        // while(1)
        // {
            if(msgsnd(id, &x, 1025,0) == -1)
            {
                perror("wysylanie wiadmosci 1");
                exit(1);
            }

            // if(msgrcv(id, &x, 7, 0, IPC_NOWAIT) == -1)
            // {
            //     perror("odbieranie wiadomosci 1");
            //     exit(1);
            // }
        // }
    }

    if(fork() == 0)
    {
        struct msgbuf y;
        y.mtype = 10;
        // strcpy(y.mtext, "Hello2\n");
        // char *msg[1025];
        // while(1)
        // {
            if(msgrcv(id, &y, 1025, 10, 0) == -1)
            {
                perror("odbieranie wiadomosci 2");
                exit(1);
            }

            // write(1, y.mtext, 1025);
            printf("%s\n", y.mtext);

            // if(msgsnd(id, &y, 7, IPC_NOWAIT) == -1)
            // {
            //     perror("wysylanie wiadmosci 2");
            //     exit(1);
            // }
        // }
    }
    // while(1);
    return 0;
}