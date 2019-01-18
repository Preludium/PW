#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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
#include <pthread.h>

const int SIZE = 1000000;
int tab[SIZE];

void *start_routine(void *argument) 
{
    int *p = (int*) argument;
    int x = 1;
    for( int i = 2; i <= *p; i++)
        x *= i;
    
    *p = x;
    return p;
}

void *licz(void *arg)
{
    int sum;
    for(int i = 0; i < SIZE; i++)
        sum += tab[i];


}

int main()
{
    pthread_t id[10];
    for(int i = 0; i < SIZE; i++)
        tab[i] = rand() %10;

    if (pthread_create(&id, NULL, licz, NULL) != 0)
    {
        perror("creating thread");
        exit(-1);
    }

    return 0;
}


    // int arg = 5;
    // void *cos;

    // if (pthread_create(&id, NULL, start_routine, &arg) != 0)
    // {
    //     perror("creating thread");
    //     exit(-1);
    // }

    // if(pthread_join(id, &cos) != 0)
    // {
    //     perror("join failed");
    //     exit(-1);
    // }

    // printf("silnia main = %d\n", *(int*)cos);
    // sleep(2);