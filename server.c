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

#define MAX 100

int main()
{
    char server_name[] = "ser_fifo", client_name[MAX], command[MAX];
    int fds, fdc;
    int i, j;
    char * args[MAX];
    unlink(server_name);

    if(mkfifo(server_name, S_IRUSR | S_IWUSR) == -1)
    {
        perror("server fifo create error");
        exit(1);
    }
    
    while(1)
    {
        if((fds = open(server_name, O_RDWR)) == -1)
        {
            perror("server fifo open error");
            exit(1);
        } 
        read(fds, client_name, sizeof(client_name));
        //unlink(client_name);

        if((fdc = open(client_name, O_RDWR)) == -1)
        {
            perror("client fifo open error");
            exit(1);
        }
        read(fdc, command, sizeof(command));

        i = 0;
        args[0] = &command[0];
        j = 1;
        while(command[i] != '\n')
        {
            if(command[i] == ' ')
            {
                args[j] = &command[i+1];
                command[i] = '\0';
                j++;
            }
            i++;
        }
        command[i] = '\0';
        args[j] = NULL;  

        dup2(fds, 0);
        if(fork() == 0)
        {
            execvp(args[0], args);
            exit(0);
        }      
    }
    return 0;   
}