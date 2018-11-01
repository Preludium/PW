#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int a = open(argv[1], O_RDWR);
	char buf[512];
	int n, i;
	char temp;
	
	if(a == -1)
	{
		perror("File Opening");	
		exit(1);
	}
	
	while((n = read(a, buf, 512)) > 0)
	{
		i = 0;
		while(buf[i] != '\n')
		{
			i++;
		}
		
		for(int j = 0; j < i/2; j++)
		{
			temp = buf[j];
			buf[j] = buf[i-1-j];
			buf[i-1-j] = temp;
		}
		
		lseek(a, -n, SEEK_CUR);

		if(write(a, buf, i+1) == -1)
		{
			perror("Write error");
			exit(1);
		}
	}

	if (n == -1) 
        {
            perror("Read error");
            exit(1);
        }
	
	close(a);
	return 0;
}
