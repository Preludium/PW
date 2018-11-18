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
    char server_name[] = "ser_fifo", client_name[] = "Kupa", command[MAX];
    int fds, fdc;
    while(1)    
    {
        //printf("Client fifo name : ");
        //scanf("%s", client_name);

        unlink(client_name);
        if(mkfifo(client_name, S_IRUSR | S_IWUSR) == -1)
        {
            perror("client fifo create error");
            exit(1);
        }  

        printf("Command : ");
        fgets(command, MAX, stdin);

        if((fdc = open(client_name, O_RDWR)) == -1)
        {
            perror("client fifo open error");
            exit(1);
        }  
        write(fdc, command, MAX);

        if((fds = open(server_name, O_RDWR)) == -1)
        {
            perror("server fifo open error");
            exit(1);
        } 
        write(fds, client_name, MAX);   
        return 0; 
    }
    return 0;
}