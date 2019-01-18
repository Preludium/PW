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
    int fds, fdc, n;
    char buf[MAX], message[2 * MAX - 1];

    //      ustalanie nazwy fifo clienta
    printf("Client fifo name : ");
    fgets(client_name, sizeof(client_name), stdin);

    if ((strlen(client_name) > 0) && (client_name[strlen (client_name) - 1] == '\n'))
        client_name[strlen (client_name) - 1] = '\0';

    unlink(client_name);

    //      tworzenie fifo clienta
    if(mkfifo(client_name, S_IRUSR | S_IWUSR) == -1)
    {
        perror("client fifo create error");
        exit(1);
    }  

    while(1)    
    {
        //      ustalanie przekazywanej komendy
        printf("> ");
        fgets(command, sizeof(command), stdin);

        if(strcmp(command,"exit\n") == 0)
        {
            return 0;
        }
        else if(strcmp(command,"\n") == 0)
        {
            continue;
        }
        else
        {
            //      kodowanie wiadomosci przekazywanej do servera
            strcpy(message, client_name);
            strcat(message, "#");
            if ((strlen(command) > 0) && (command[strlen (command) - 1] == '\n'))
                command[strlen (command) - 1] = '\0';
            strcat(message, command);

            //      pisanie do fifo servera
            if((fds = open(server_name, O_WRONLY)) == -1)
            {
                perror("server fifo open error");
                exit(1);
            } 
            write(fds, message, sizeof(message)); 
            close(fds);

            //      czytanie z fifo clienta
            if((fdc = open(client_name, O_RDONLY)) == -1)
            {
                perror("client fifo open error");
                exit(1);
            }  

            while((n = read(fdc, buf, sizeof(buf))) > 0) 
            {
                write(1, buf, n);
            }    
            close(fdc); 
        }
    }
    return 0;
}
