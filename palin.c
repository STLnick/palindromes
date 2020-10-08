#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

void criticalsection(FILE *fptr, int row, char str[]);
int detach(int shmid, void *shmaddr);
int is_palindrome(char str[]);
void logfilewrite(FILE *fptr, int row, char str[]);
void resultwrite(FILE *fptr, char str[]);

/* * argv[0] = "palin" executable
   * argv[1] = id of shared memory array
   * argv[2] = id of shared memory choosing array
   * argv[3] = id of shared memory number array
   * argv[4] = index of string in shared memory array to test & identifier for process in 'choosing' and 'number' shared arrays
   * argv[5] = max size of strings
   * argv[6] = number of processes that will be ran */

int main(int argc, char **argv)
{
  FILE *fptr;          // File pointer to write to palin, nopalin and logfile
  char *sharedstrings; // Shared memory array for strings
  int *choosing;      // Shared memory choosing array
  int *number;         // Shared memory number array
  int i;               // Loop counter

  int id = atoi(argv[1]);
  int choosingid = atoi(argv[2]);
  int numberid = atoi(argv[3]);
  int index = atoi(argv[4]);
  int strsize = atoi(argv[5]);
  int num_procs = atoi(argv[6]);

  // Attach to the allocated shared memory segments
  if ((sharedstrings = (char *) shmat(id, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(id, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  if ((choosing = (int *) shmat(choosingid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(choosingid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  if ((number = (int *) shmat(numberid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(numberid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }


  char buffer[strsize]; // Buffer to store word as string
  int nullfound = 0;    // Flag if \0 is found

  // Extract string to test from shared array
  for (i = 0; i < strsize; i++)
  {
    if (!nullfound)
    {
      buffer[i] = sharedstrings[index * strsize + i];
      if (sharedstrings[index * strsize + i] == '\0')
        nullfound = 1;
    }
  }

  /* * CODE TO ENTER C.S. * */
  choosing[index] = 1;
  int max = 0;
  for(i = 0; i < num_procs; i++)
  {
    if (number[i] > max)
      max = number[i];
  }
  number[index] = max + 1;
  choosing[index] = 0;

  for (i = 0; i < num_procs; i++)
  {
    while (choosing[i] != 0);
    while ((number[i] != 0) && (number[i] < number[index] || (number[i] == number[index] && i < index)));
  }

  /* * * * * * * * * * * * */
  /* * CRITICAL SECTION * */
  /* * * * * * * * * * * * */

  criticalsection(fptr, index, buffer);

  /* * * * * * * * * * * * * * */
  /* * END CRITICAL SECTION  * */
  /* * * * * * * * * * * * * * */

  // Reset number in shared number array
  number[index] = 0;

  // detach from shared memory segments
  detach(id, sharedstrings);
  detach(choosingid, choosing);
  detach(numberid, number);

  return 0;
}

void criticalsection(FILE *fptr, int row, char str[])
{
  sleep(2);                     // Delay execution by 2 seconds
  resultwrite(fptr, str);       // Open, write to, and close either palin or nopalin depending on result
  logfilewrite(fptr, row, str); // Open, write to, and close logfile
}

// Check if supplied string is a palindrome
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

// Open, write to, and close output.log file
void logfilewrite(FILE *fptr, int row, char str[])
{
  fptr = fopen("output.log", "a");
  fprintf(fptr, "%i %i %s\n", getpid(), row, str);
  fclose(fptr);
}

// Open, write to, and close either palin.out or nopalin.out file
void resultwrite(FILE *fptr, char str[])
{
  fptr = is_palindrome(str) ? fopen("./palin.out", "a") : fopen("./nopalin.out", "a");
  fprintf(fptr, "%s\n", str);
  fclose(fptr);
}
