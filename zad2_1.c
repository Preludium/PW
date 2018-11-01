#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int a = open(argv[1], O_RDONLY);
	char buf[512];
	int n, i, max = 0, num, j = 1,x;
	
	if(a == -1)
	{
            perror("File Opening");	
            exit(1);
	}
	
	while((n = read(a, buf, 512)) > 0)
	{
            x = 0;
            while(x != n)
            {
                i = 0;
                while(buf[x] != '\n')
                {
                    i++;
                    x++;
                }
                if(i > max)
                {
                    max = i;
                    num = j;
                }
		
                lseek(a, 1, SEEK_CUR);
                x++;
                j++;
            }
	}
        
        if (n == -1) 
        {
            perror("Read error");
            exit(1);
        }
        
        printf("Longest line : %d, amount of characters : %d\n", num, max);
	
	close(a);
	return 0;
}