/*
EPI GIJÓN
GRADO EN INGENIERIA INFORMATICA
SISTEMAS DISTRIBUIDOS - CURSO 3º
MATERIAL DE LA ASIGNATURA
-------------------------
MECANISMO   : PIPES
FICHERO     : prodcons.c
DESCRIPCION : Se propone la realizacion de un programa en el que dos
procesos se envien a traves de un pipe un fichero de texto donde 
el que recibe debe mostrarlo por su salida standard.
*/

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


void 
Error(const char * error) {
  perror(error);
  exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[])
{
  int pipefd[2];
  pid_t child_producer = 0, child_consumer = 0;
  int file = -1;
  ssize_t count = -1;
  unsigned char data = 0;


  /* Create the pipe */
  if (pipe(pipefd) == -1)
    Error("pipe");
                  
  /* Check for correct number of arguments */
  if (argc != 2) {
    printf("Usage: prodcons <file>\n");
    exit(EXIT_FAILURE);
  }
  
  /* Create producer child */
  child_producer = fork();

  if (child_producer == -1)
    Error("Child_producer");
  
  /* Producer child's stuff */
  else if (child_producer == 0) {

    if ((file = open(argv[1], O_RDONLY)) == -1)
      Error("Open file");

    if (close(pipefd[STDIN_FILENO]) == -1)
      Error("Close stdin");
  
    while ((count = read(file, &data, sizeof(data))) != 0) {
      if (write(pipefd[STDOUT_FILENO], &data, sizeof(data)) == -1)
        Error("Write pipefd[STDOUT_FILENO]");
    }

    if (close(pipefd[STDOUT_FILENO]) == -1)
      Error("Close pipefd[STDOUT_FILENO]");

    if (close(file) == -1)
      Error("Close file");

  }
    
  /* Parent creates second child */
  else {

    /* Create consumer child */
    child_consumer = fork();

    if (child_consumer == -1) 
      Error("Child_consumer");
    
    /* Consumers child's stuff */ 
    else if (child_consumer == 0) {

      if (close(pipefd[STDOUT_FILENO]) == -1)
        Error("Close stdout");
       
      while ((count = read(pipefd[STDIN_FILENO], &data, sizeof(data))) != 0) {
        if (write(STDOUT_FILENO, &data, sizeof(data)) == -1)
          Error("Write STDOUT");
      }

      if (close(pipefd[STDIN_FILENO]) == -1)
        Error("Close pipe[STDIN_FILENO]");
    }

    else {
      //wait(0);
      printf("padre?");
    
            
      if (close(pipefd[STDIN_FILENO]) == -1 || close(pipefd[STDOUT_FILENO] == -1))
        Error("Close Parent Pipes");
      
      pid_t childs[2] = {child_consumer, child_producer};

      //Se tarda tanto que el hijo 0 muere sin que el padre pueda hacer nada
      int status;
      
      //waitpid(childs[1], &status, 0);
      printf("exit status %d = %d\n", 0, WEXITSTATUS(status));
      



      /*    
      waitpid(childs[1], &status, 0);
      printf("exit status %d = %d\n", 1, WEXITSTATUS(status));
      */

      /*
      if (wait(0) == -1 || wait(0) == -1) 
        Error("Wait");
      */
    }
  }
  return 0;
}
