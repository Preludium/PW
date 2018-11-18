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
    
    while(1)    
    {
        printf("Client fifo name : ");
        fgets(client_name, sizeof(client_name), stdin);

        if ((strlen(client_name) > 0) && (client_name[strlen (client_name) - 1] == '\n'))
            client_name[strlen (client_name) - 1] = '\0';

        unlink(client_name);

        if(mkfifo(client_name, S_IRUSR | S_IWUSR) == -1)
        {
            perror("client fifo create error");
            exit(1);
        }  
        
        printf("Command : ");
        fgets(command, sizeof(command), stdin);

        // if ((strlen(command) > 0) && (command[strlen (command) - 1] == '\n'))
        //     command[strlen (command) - 1] = '\0';

        // printf("%s\n", command);


        if((fdc = open(client_name, O_RDWR)) == -1)
        {
            perror("client fifo open error");
            exit(1);
        }  
        write(fdc, command, MAX);        

        // free(command);

        if((fds = open(server_name, O_RDWR)) == -1)
        {
            perror("server fifo open error");
            exit(1);
        } 
        write(fds, client_name, MAX); 

        // return 0;

        // if((fdc = open(server_name, O_RDWR)) == -1)
        // {
        //     perror("server fifo open error");
        //     exit(1);
        // } 
        // read(fdc, stdout, MAX);
        return 0; 
    }
    return 0;
}