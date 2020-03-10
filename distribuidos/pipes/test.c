
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
    int fds[2];
    pipe(fds);
    int file = -1;
    ssize_t count = -1;
    unsigned char data = 0;

    if (fork() == 0) {
      
      file = open(argv[1], O_RDONLY);
      close(fds[STDIN_FILENO]);
  
      while ((count = read(file, &data, sizeof(data))) != 0) {
        write(fds[STDOUT_FILENO], &data, sizeof(data));
      }

      close(fds[STDOUT_FILENO]);
      close(file);
    }

    else if ( fork() == 0) {

      close(fds[STDOUT_FILENO]);
       
      while ((count = read(fds[STDIN_FILENO], &data, sizeof(data))) != 0) {
        write(STDOUT_FILENO, &data, sizeof(data));
      }

      close(fds[STDIN_FILENO]);
    }

    else {
      close(fds[0]);
      close(fds[1]);
      wait(0);
      wait(0);
    }

  return 1;
}