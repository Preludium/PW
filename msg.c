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
    int mdane;
};

int main()
{
    int id;
    if((id = msgget(0x250, IPC_CREAT | 0600)) == -1)
    {
        perror("Create message queue error");
        exit(1);
    }

    if(fork() == 0)
    {
        struct msgbuf x;
        x.mtype = 10;
        while(1)
        {
            printf("Enter the number : ");
            scanf("%d", &x.mdane);
            
            if(msgsnd(id, &x, sizeof(int), 0) == -1)
            {
                perror("Client send message error");
                exit(1);
            }
            
            if(x.mdane == 0)
            {
                if(msgrcv(id, &x, sizeof(int), 10, 0) == -1)
                {
                    perror("Client receive message error");
                    exit(1);
                }
                printf("The sum is %d\n", x.mdane);
                exit(0);
            }
        }
    }

    if(fork() == 0)
    {
        struct msgbuf y;
        y.mtype = 10;
        int sum = 0;
        while(1)
        {
            if(msgrcv(id, &y, sizeof(int), 10, 0) == -1)
            {
                perror("Server receive message error");
                exit(1);
            }

            if(y.mdane != 0)
                sum += y.mdane;
            else
            {
                y.mdane = sum;
                if(msgsnd(id, &y, sizeof(int), 0) == -1)
                {
                    perror("Server send message error");
                    exit(1);
                }
                exit(0);
            }
        }
    }
    wait(NULL);
    wait(NULL);
    return 0;
}