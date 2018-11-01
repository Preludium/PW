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

int main(int argc, char * argv[])
{
    if(argc == 1 || argc == 3)
    {
    int j = 0;
    char prog[sizeof(argv[0])];
    for(int i = 2; i <= sizeof(argv[0]);i++)
    {
        prog[j]=*(argv[0]+i);
        j++;
    }
    if (argc == 1)
    {
        printf("%s\n", prog);
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], prog) == 0)
        {
            printf("%s\n", argv[2]);
        }
        else
        {
	    execlp(argv[1], argv[2], NULL);
        }
    }
    }
    else printf("Bledna ilosc argumentow\n");
    return 0;   
}
