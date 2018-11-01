#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
	int a = open(argv[1], O_RDONLY);
	int b = open(argv[2], O_CREAT | O_WRONLY);
	int n;
	char buf[64];
	
        if (a == -1)
        {
            perror("File opening");
            exit(1);
        }
        
	while ((n = read(a, buf, 64)) > 0)
	{
            if (write(b,buf,n) == -1)
            {
                perror("write error");
                exit(1);
            }
        }
        
        if (n == -1) 
        {
            perror("read error");
            exit(1);
        }
        
	close(a);
	close(b);
	return 0;
}
