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
  char name[] = "myfifo", name1[] = "myfifo1";
  int fd, fd1;
  unlink(name), unlink(name1); 
  
  if (mkfifo(name, S_IRUSR | S_IWUSR) == -1) // nie mozna dwa razy do tego same "name" sie odwolywac | uzywac unlink
  {
    perror("fifo create error");
    exit(1);
  }
  
  if(mkfifo(name1, S_IRUSR | S_IWUSR) == -1)
  {
    perror("fifo create error");
    exit(1);
  }
  
  if (fork() == 0) 
  {
    if ((fd = open(name, O_WRONLY)) == -1) 
    {
      perror("child fifo open error");
      exit(1);
    }
    dup2(fd, 1);
    execlp("ls", "ls", "-l", NULL);
  }
  else if(fork() == 0)
  {
    if ((fd = open(name, O_RDONLY)) == -1) 
    {
      perror("child fifo open error");
      exit(1);
    }
    
    if ((fd1 = open(name1, O_WRONLY)) == -1) 
    {
      perror("child fifo open error");
      exit(1);
    } 
    dup2(fd, 0);
    dup2(fd1, 1);
    execlp("grep", "grep", "^d", NULL);
    exit(0);
  }
  else
  {
    if ((fd1 = open(name1, O_RDONLY)) == -1) 
    {
      perror("child fifo open error");
      exit(1);
    }
    dup2(fd1, 0);
    execlp("more", "more", NULL);
    exit(0);
  }
  return 0;
}
