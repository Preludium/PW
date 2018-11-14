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
    char name[] = "ser_fifo";
    int fd;
    
    while(true)
    {
        if(mkfifo(name1, S_IRUSR | S_IWUSR) == -1)
        {
            perror("fifo create error");
            exit(1);
        } 
    
        if (fork() == 0) 
        {
            if ((fd = open(name, O_RDONLY)) == -1) 
            {
                perror("child fifo open error");
                exit(1);
            }
            dup2(fd, 1);
            execlp("ls", "ls", "-l", NULL);
        }
        
        
    }
     return 0;   
}
