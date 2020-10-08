#define _XOPEN_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>

char** buildstringarray(int *indexcount);
int detachandremove(int shmid, void *shmaddr);
void displayhelpinfo();

static void myhandler(int s)
{
  // Free shared memory
  
  // Kill children processes
  kill(0, SIGINT);
}

static int setupinterrupt()
{
  struct sigaction act;
  act.sa_handler = myhandler;
  act.sa_flags = 0;
  return (sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
}

static int setupitimer(int time) 
{
  struct itimerval value;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  value.it_value.tv_sec = time;
  value.it_value.tv_usec = 0;

  return(setitimer(ITIMER_REAL, &value, NULL));
}

#define STR_SIZE 80

int main (int argc, char **argv)
{
  int childpid;                  // PID of a child process
  int id;                        // ID for the shared memory array segment
  key_t key = ftok("/tmp", 'K'); // Key to allocate shared memory segment for array

  /* Values specified by command line arguments - defined below */
  int max_proc, max_children, opt, run_time;

  int child_cnt = 0;                   // Counter for currently running children processes
  int nflag = 0, sflag = 0, tflag = 0; // Flags for command line options

  int num_strings = 0; // Holds number of strings read in
  char **strings;      // Holds strings read in to copy into shared memory
  char *sharedstrings; // Array to hold strings in shared memory

  // Parse command line options
  while ((opt = getopt(argc, argv, "hn:s:t:")) != -1)
  {
    switch (opt)
    {
      case 'h': // Help - describe how to run program and default values
        displayhelpinfo();
        exit(EXIT_SUCCESS);
        break; 
      case 'n': // Specify number of processes to be ran in total
        nflag = 1;
        max_proc = atoi(optarg);
        break;
      case 's': // Specify number of children to be running at any one time
        sflag = 1;
        max_children = atoi(optarg);
        break;
      case 't': // Specify number of seconds the program will run before terminating
        tflag = 1;
        run_time = atoi(optarg);
        break;
      default:
        printf("Please use -h for help to see valid options.\n");
        exit(EXIT_FAILURE);
    }
  }


  // Set default values for unused options
  max_proc = nflag ? max_proc : 4;
  max_children = sflag ? max_children : 2;
  run_time = tflag ? run_time : 100;

  // Enforce hard limits on options
  max_proc = max_proc > 20 ? 20 : max_proc;
  max_proc = max_proc > 0 ? max_proc : 4;
  max_children = max_children > 5 ? 5 : max_children;
  max_children = max_children > 0 ? max_children : 2;
  run_time = run_time > 120 ? 120 : run_time;
  run_time = run_time > 0 ? run_time : 100;

  // Setup alarm interrupt handler
  if (setupinterrupt() == -1)
  {
    perror("Failed to setup SIGALRM handler");
    return 1;
  }

  // Setup timer
  if (setupitimer(run_time) == -1)
  {
    perror("Failed to setup timer");
    return 1;
  }

  strings = buildstringarray(&num_strings); // Read in strings from stdin to array and store number read in

  // Allocate shared memory segment for array
  if ((id = shmget(key, sizeof(char *) * STR_SIZE * num_strings, IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  // Attach to the allocated shared memory for array
  if ((sharedstrings = (char *) shmat(id, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(id, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  // Copy strings into shared memory array
  int i, j;       // For loop counters
  int srcstrsize; // Holds size of string from 'strings'
  for (i = 0; i < num_strings; i++)
  {
    srcstrsize = strlen(strings[i]);
    for (j = 0; j < STR_SIZE; j++)
    {
      if (strings[i][j] == '\n')
        sharedstrings[i * STR_SIZE + j] = '\0';
      else if (j >= srcstrsize)
        sharedstrings[i * STR_SIZE + j] = '\0';
      else
        sharedstrings[i * STR_SIZE + j] = strings[i][j];
    }  
  }

  int wpid; // PID of process parent just waited on

  // Max number of time loop will run and in turn the max number of processes that will be ran
  int maxiterations = max_proc > num_strings ? num_strings : max_proc;

  int choosingid, numberid;
  int *choosing;
  int *number;

  // Allocate shared 'choosing' array
  if ((choosingid = shmget(key, sizeof(int) * maxiterations, IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  // Attach to 'choosing' memory segment
  if ((choosing = (int *) shmat(choosingid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(choosingid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  // Allocate shared 'number' array
  if ((numberid = shmget(key, sizeof(int) * maxiterations, IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  // Attach to 'number' memory segment
  if ((number = (int *) shmat(numberid, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(numberid, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  // Initialize values of 'choosing' and 'number'
  for (i = 0; i < maxiterations; i++)
  {
    choosing[i] = 0;
    number[i] = 0;
  }

  /* * * * * * * * * * */
  /* * PROCESS LOOP  * */
  /* * * * * * * * * * */

  for (i = 0; i < maxiterations; i++)
  {
    // Fork fail check
    if ((childpid = fork()) == -1)
    {
      perror("Failed to create child process.");
      if (detachandremove(id, sharedstrings) == -1)
        perror("Failed to detach and remove shared memory segment.");
    }
    child_cnt++; // Increase count of children currently running

    // Child Code
    if (childpid == 0)
    {
      char strid[100+1] = {'\0'}; // Create string from shared memory array id
      sprintf(strid, "%d", id);

      char str_choosingid[100+1] = {'\0'}; // Create string from shared memory choosing array id
      sprintf(str_choosingid, "%d", choosingid);

      char str_numberid[100+1] = {'\0'}; // Create string from shared memory number array id
      sprintf(str_numberid, "%d", numberid);

      char str_procindex[100+1] = {'\0'}; // Create string from loop index to identify each process
      sprintf(str_procindex, "%d", i);

      char strmaxsize[100+1] = {'\0'}; // Create string from max string size
      sprintf(strmaxsize, "%d", STR_SIZE);

      char strnumprocs[100+1] = {'\0'}; // Create string from number of processes to be ran
      sprintf(strnumprocs, "%d", maxiterations);

      // Create custom argv array to exec 'palin' with
      char *args[8] = {"palin", strid, str_choosingid, str_numberid, str_procindex, strmaxsize, strnumprocs, '\0'};

      /* * argv[0] = "palin" executable
         * argv[1] = id of shared memory array
         * argv[2] = id of shared memory choosing array
         * argv[3] = id of shared memory number array */
      execv(args[0], args);
      perror("Child failed to exec");
    }

    if (child_cnt == max_children)
    {
      wait(NULL);
      child_cnt--;
    }
  }

  // Wait for remaining child processes
  while (child_cnt != 0)
  {
    wpid = wait(NULL);
    printf("Parent waited for pid: %i\n", wpid);
    child_cnt--; // Decrement count of children currently running
  }

  // Detach from and remove the shared memory segments
  detachandremove(id, sharedstrings);
  detachandremove(choosingid, choosing);
  detachandremove(numberid, number);
  
  return 0;
}

// Detach from and delete the shared memory segment
int detachandremove(int shmid, void *shmaddr)
{
  int error = 0;

  if (shmdt(shmaddr) == -1)
    error = errno;
  if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
    error = errno;
  if (!error)
  {
    printf("Successfully detached and removed the shared memory segment - id: %d\n", shmid);
    return 0;
  }
  errno = error;
  perror("Error: ");
  return -1;
}

// Builds and returns string array to copy into shared memory and by reference variable determines number of strings in array
char** buildstringarray(int *indexcount) {
  char buf[20];                            // Buffer to hold each string read from stdin
  char **strings = malloc(sizeof(char *)); // Dynamically built array to hold all strings from stdin

  fgets(buf, 20, stdin);             // Get first string from stdin
  strings[*indexcount] = malloc(20); // Use malloc for element - will use realloc in loop
  strcpy(strings[*indexcount], buf); // Copy read string into dynamic array
  *indexcount += 1;                  // Increment index counter

  // Get rest of strings from input file
  while(fgets(buf, 20, stdin))
  {
    strings = realloc(strings, sizeof(*strings) * (*indexcount + 1)); // Increase size of dynamic array
    strings[*indexcount] = malloc(20);                                // Allocate new element's memory space
    strcpy(strings[*indexcount], buf);                                // Copy read string into dynamic array
    *indexcount += 1;                                                 // Increment index counter
  }

  return strings;
}

void displayhelpinfo()
{
  printf("\nPalindrome master program\n");
  printf("-------------------------\n");
  printf("Example command to run ./master:\n\n");
  printf("./master -n 10 -s 4 -t 60 < palindromes\n\n");
  printf("-------------------------\n");
  printf("Program options information:\n");
  printf("-n = The total number of processes to be ran.\n");
  printf("-s = The number of children processes to be running at any one time.\n");
  printf("-t = The time in seconds that the program will execute. This will end\n");
  printf("     execution even if the processes haven't finished the tasks.\n\n");
  printf("-------------------------\n");
  printf("Default value information:\n");
  printf("All options specified above are optional and will use the\n");
  printf("following default values if not specified.\n\n");
  printf("-n = 4\n");
  printf("-s = 2\n");
  printf("-t = 100\n\n");
  printf("NOTE: Any Negative values passed will be set to these default values");
  printf("      Any excessive values will be set to a ceiling value.");
  printf("-------------------------\n");
  printf("Program *MUST* be supplied with a file redirect '< palindromes'\n");
  printf("where 'palindromes' is a simple file with plain text and a word\n");
  printf("on each line.\n");
}

