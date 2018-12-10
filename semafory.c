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

int x;
static struct sembuf buf;

void podnies (int semid, int semnum) 
{
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    
    if (semop(semid, &buf, 1) == -1) 
    {
        perror("podnoszenie semafora");
        exit(1);
    }
}

void opusc(int semid, int semnum) 
{
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    
    if (semop(semid, &buf, 1) == -1) 
    {
        perror("opuszczenie semafora");
        exit(1);
    }
}

void kolacja(int n)
{
    if (fork() == 0)
    {
        int left, right;
        right = n;
        if(n == 0)
            left = 4;
        else
            left = n - 1;
        while(1)
        {
            opusc(x, right);
            printf("%d takes right fork\n", n+1);

            opusc(x, left);
            printf("%d takes left fork\n", n+1);

            printf("%d eats\n", n+1);
            sleep(2);

            podnies(x, right);
            printf("%d gives back right fork\n", n+1);

            podnies(x, left);
            printf("%d gives back left fork\n", n+1);
        }
    }
}

void stop () 
{
    if (semctl(x, 0, IPC_RMID, NULL) == -1) 
    {
        perror("cannot remove semaphore");
        exit(1);
    }
    exit(0);
}

int main()
{
    if ((x = semget(0x666, 5, IPC_CREAT | 0777)) == -1) 
    {
        perror("Cannot get semaphore");
        exit(1);
    }
    
    for(int i = 0; i < 5; i++)
    {   
        if (semctl(x, i, SETVAL, 1) == -1) 
        {
            perror("Cannot set semaphore's value");
            exit(1);
        }
        kolacja(i);
    }

    signal(SIGINT, stop);
    while(1);
    return 0;
}