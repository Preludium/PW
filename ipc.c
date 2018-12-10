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

// volatile sig_atomic_t global = 0; // kompilator nie bedzie nic kombinowal z ta zmienna, atomowa zmienna

int main()
{
    char *buffer;
    int shmid;

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

        if((buffer = shmat(shmid, buffer, 0)) == (void *) -1) 
        {
            perror("cannot attach shared memory segment");
            exit(1);
        }

        while(1)
            strcpy(buffer,"haslo okon\n");

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

        if ((buffer = shmat(shmid, buffer, 0)) == (void *) -1) 
        {
            perror("cannot attach shared memory segment");
            exit(1);
        }

        while(1)
            strcpy(buffer,"Hello World\n");

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
            write(1, buffer, 100);
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