#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>

int detach(int shmid, void *shmaddr);
int is_palindrome(char str[]);

/*
* argv[0] = palin
* argv[1] = id of shared memory segment
* argv[2] = (index of string in shared memory array to test i.e. the 'row' in array to start)
* argv[3] = max size of strings
*/

int main(int argc, char **argv)
{
  FILE *fptr;
  int palinresult;
  char *sharedstrings;

  int id = atoi(argv[1]);
  int row = atoi(argv[2]);
  int strsize = atoi(argv[3]);

  // Attach to the allocated shared memory
  if ((sharedstrings = (char *) shmat(id, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(id, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  int i;
  char buffer[strsize];
  int nullfound = 0;
  for (i = 0; i < strsize; i++)
  {
    if (!nullfound)
    {
      buffer[i] = sharedstrings[row * strsize + i];
      if (sharedstrings[row * strsize + i] == '\0')
        nullfound = 1;
    }
  }

  palinresult = is_palindrome(buffer);
  printf("\n");

  // TESTING - run just one process and see if this works
  if (palinresult)
    fptr = fopen("./palin.out", "a");
  else
    fptr = fopen("./nopalin.out", "a");

  fprintf(fptr, "%s", buffer);

  fclose(fptr);

  // TODO: Develop code to enter the critical section
  //       Develop the criticalsection() code
  //         - Write to palin.out or nopalin.out
  //         - Write to output.log
  //       Develop code to exit the critical section

  // detach from shared memory segment
  detach(id, sharedstrings);

  return 0;
}

int is_palindrome(char str[])
{
  int i = 0;               // Start of string index
  int j = strlen(str) - 1; // End of string index

  while (j > 1)
  {
    if (str[i++] != str[j--])
      return 0; // False
  }
  return 1; // True
}

// Detach from the shared memory segment
int detach(int shmid, void *shmaddr)
{
  int error = 0;

  if (shmdt(shmaddr) == -1)
    error = errno;

  if (!error)
    return 0;

  errno = error;
  return -1;
}
