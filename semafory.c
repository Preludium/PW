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


// struct sembuf
// {
//     short sem_num;
//     short sem_op;
//     short sem_flg;
// };

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

int main()
{
    char *buffer;
    int shmid;

    int x = semget(0x666, 1, IPC_CREAT | 0660);

    semctl(x, 1, IPC_STAT, 1);

    if ((shmid = shmget(0x113, 100, IPC_CREAT | 0660)) == -1) 
    {
        perror("cannot get shared memory segment");
        exit(1);
    }
    
    
    if (fork() == 0)
    {
        if ((shmid = shmget(0x113, 100, IPC_CREAT | 0660)) == -1) 
        {
            perror("cannot get shared memory segment");
            exit(1);
        }

        if((buffer = shmat(shmid, NULL, 0)) == (void *) -1) 
        {
            perror("cannot attach shared memory segment");
            exit(1);
        }

        while(1)
        {
            podnies(x, 0);
            strcpy(buffer,"Murinho_dobry_trener\n");
            opusc(x, 0);
        }
        if (shmdt((const void *) buffer) == -1) 
        {
            perror("cannot dettach shared memory segment");
            exit(1);
        }
            
        if (shmctl(shmid, IPC_RMID, NULL) == -1) 
        {
            perror("cannot remove shared memory segment");
            exit(1);
        }

        exit(0);
    }

    if (fork() == 0)
    {
        if ((shmid = shmget(0x113, 100, IPC_CREAT | 0660)) == -1) 
        {
            perror("cannot get shared memory segment");
            exit(1);
        }

        if ((buffer = shmat(shmid, NULL, 0)) == (void *) -1) 
        {
            perror("cannot attach shared memory segment");
            exit(1);
        }

        while(1)
        {
            podnies(x, 0);
            strcpy(buffer,"Hello World\n");
            opusc(x, 0);
        }
        if (shmdt((const void *) buffer) == -1) 
        {
            perror("cannot dettach shared memory segment");
            exit(1);
        }

        if (shmctl(shmid, IPC_RMID, NULL) == -1) 
        {
            perror("cannot remove shared memory segment");
            exit(1);
        }

        exit(0);
    }        

    
        if ((buffer = shmat(shmid, NULL, 0)) == (void *) -1) 
        {
            perror("cannot attach shared memory segment");
            exit(1);
        }

        while(1)
        {
            podnies(x, 0);
            write(1, buffer, 100);
            opusc(x, 0);
            sleep(2);
        }
        if (shmdt((const void *) buffer) == -1) 
        {
            perror("cannot dettach shared memory segment");
            exit(1);
        }
    
        if (shmctl(shmid, IPC_RMID, NULL) == -1) 
        {
            perror("cannot remove shared memory segment");
            exit(1);
        }
        
        sleep(2);
}