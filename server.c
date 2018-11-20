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
    char server_name[] = "ser_fifo", client_name[MAX], message[2 * MAX - 1];
    int fds, fdc;
    int i, j;
    char *args[MAX];

    //      tworzenie fifo servera
    unlink(server_name);
    if(mkfifo(server_name, S_IRUSR | S_IWUSR) == -1)
    {
        perror("server fifo create error");
        exit(1);
    }
    
    while(1)
    {
        //      czytanie wiadomosci z fifo servera
        printf("opening server\n");
        if((fds = open(server_name, O_RDONLY)) == -1)
        {
            perror("server fifo open error");
            exit(1);
        } 
        printf("reading from server\n");
        read(fds, message, sizeof(message));
        close(fds);

        //      odkodowywanie wiadomosci
        i = 0;
        j = 1;
        while(message[i] != '#')
        {
            client_name[i] = message[i];
            i++;
        }
        client_name[i] = '\0';

        i++;
        args[0] = &message[i];
        while(message[i] != '\0')
        {
            if(message[i] == ' ')
            {
                args[j] = &message[i+1];
                message[i] = '\0';
                j++;
            }
            i++;
        }
        args[j] = NULL;

        //      pisanie do fifo clienta
        if((fdc = open(client_name, O_WRONLY)) == -1)
        {
            perror("client fifo open error");
            exit(1);
        }

        dup2(fdc, 1);

        if(fork() == 0)
        {
            if(execvp(args[0], args) == -1)
            {
                printf("Command \" ");
                for(int l=0; l<j;l++)
                    printf("%s ",args[l]);
                printf("\" not found\n");
                exit(1);
            }
            exit(0);
        }    
        close(1); 
        close(fdc);

        if (wait(NULL) == -1) 
        {
            perror("wait on child");
            exit(1);
        }

    }
    return 0;   
}