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

#define MAX 1025

int main()
{
    char server_name[] = "ser_fifo", client_name[MAX], command[MAX];
    int fds, fdc;
    int i, j, status;
    char * args[MAX];
    unlink(server_name);

    if(mkfifo(server_name, S_IRUSR | S_IWUSR) == -1)
    {
        perror("server fifo create error");
        exit(1);
    }
    
    while(1)
    {
        printf("otwiera server\n");
        if((fds = open(server_name, O_RDWR)) == -1)
        {
            perror("server fifo open error");
            exit(1);
        } 
        printf("czyta z servera\n");
        read(fds, client_name, sizeof(client_name));
        // close(fds);
        // unlink(client_name);
        printf("otwiera clienta %s\n", client_name);
        if((fdc = open(client_name, O_RDWR)) == -1)
        {
            perror("client fifo open error");
            exit(1);
        }
        printf("czyta z clienta\n");
        read(fdc, command, sizeof(command));
        printf("nie zesralo sie tym razem\n");
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
        printf("dup2\n");
        dup2(fds, fdc);
        if(fork() == 0)
        {
            execvp(args[0], args);
            exit(-1);
        }     
        wait(&status);
        if(WEXITSTATUS(status) != 0)
        {
            printf("Command \" ");
            for(int l=0; l<j;l++)
                printf("%s ",args[l]);
            printf("\" not found\n");
        } 
        // memset(client_name, 0, sizeof(client_name));    
    }
    return 0;   
}