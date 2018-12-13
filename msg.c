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

#define M_Dane 1
#define M_End 2
#define M_Wynik 3

struct msgbuf 
{
    long mtype;
    int mdane;
};

int main()
{
    int id;
    if((id = msgget(0x123, IPC_CREAT | 0600)) == -1)
    {
        perror("Create message queue error");
        exit(1);
    }

    if(fork() == 0)
    {
        struct msgbuf x;
        while(1)
        {
            printf("Enter the number : ");
            scanf("%d", &x.mdane);

            if(x.mdane != 0)
                x.mtype = M_Dane;
            else
                x.mtype = M_End;

            if(msgsnd(id, &x, sizeof(int), 0) == -1)
            {
                perror("Client send message error");
                exit(1);
            }
            
            if(msgrcv(id, &x, sizeof(int), 0, 0) == -1)
            {
                perror("Client receive message error");
                exit(1);
            }
 
            if(x.mtype == M_Dane)
                printf("The sum is %d\n", x.mdane);
            else if(x.mtype == M_Wynik)
            {
                printf("The sum is %d\n", x.mdane);
                exit(0);
            }
        }
    }

    if(fork() == 0)
    {
        struct msgbuf y;
        int sum = 0;
        while(1)
        {
            if(msgrcv(id, &y, sizeof(int), 0, 0) == -1)
            {
                perror("Server receive message error");
                exit(1);
            }

            if(y.mtype == M_Dane)
            {
                sum += y.mdane;
                y.mdane = sum;
                if(msgsnd(id, &y, sizeof(int), 0) == -1)
                {
                    perror("Server send message error");
                    exit(1);
                }
            }
            else if(y.mtype == M_End)
            {
                y.mdane = sum;
                y.mtype = M_Wynik;
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