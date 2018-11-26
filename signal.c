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

int global;

void handler()
{
    global++;
    global %= 2;
}

int main()
{
    global = 0;
    while(1)
    {
        signal(SIGINT, handler);
        sleep(3);
        if(fork() == 0)
        {
            if(global == 0)
            {
                execlp("ls", "ls", NULL);
            }
            else
            {
                execlp("ps", "ps", NULL);
            }
        }
    }
}