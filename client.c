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

int main()
{
    char name[] = "myfifo";
    int status;
    int j, i;
    char * args[MAX];
    bool switchon = true;
    char command[MAX];
    while(switchon == true)    
    {
        
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
            
            if(fork() == 0)
            {
                execvp(args[0], args);
                exit(-1);
            }
            else
            {
                wait(&status);
                if(WEXITSTATUS(status) != 0)
                {
                    printf("Command \" ");
                    for(int l=0; l<j;l++)
                        printf("%s ",args[l]);
                    printf("\" not found\n");
                }
            }
        }
    }
    return 0;
}
