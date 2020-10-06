#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

char** buildstringarray(int *indexcount);
int detachandremove(int shmid, void *shmaddr);
void displayhelpinfo();

#define STR_SIZE 20

int main (int argc, char **argv)
{
  int childpid;                  // PID of a child process
  int id;                        // ID for the shared memory array segment
  key_t key = ftok("/tmp", 'K'); // Key to allocate shared memory segment for array

  /* Values specified by command line arguments - defined below */
  int num_proc, num_children, opt, run_time;

  int child_cnt = 0;                   // Counter for currently running children processes
  int nflag = 0, sflag = 0, tflag = 0; // Flags for command line options

  int num_strings = 0; // Holds number of strings read in
  char **strings;      // Holds strings read in to copy into shared memory
  char *sharedstrings; // Array to hold strings in shared memory

  strings = buildstringarray(&num_strings); // Read in strings to array and store number read in

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
        num_proc = atoi(optarg);
        break;
      case 's': // Specify number of children to be running at any one time
        sflag = 1;
        num_children = atoi(optarg);
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
  num_proc = nflag ? num_proc : 4;
  num_children = sflag ? num_children : 2;
  run_time = tflag ? run_time : 100;



  // Allocate shared memory segment for array
  if ((id = shmget(key, sizeof(char *) * STR_SIZE * num_strings, IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  // TESTING MESSAGE IF WE SUCCESSFULLY GET SHARED MEMORY
  printf("Successfully allocated shared memory with id: %d\n", id);



  // Attach to the allocated shared memory
  if ((sharedstrings = (char *) shmat(id, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(id, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }
  printf("Successfully attached to the shared memory segment\n");



  // TODO: Place strings into sharedstrings for all processes to access

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
  int maxiterations = num_proc > num_strings ? num_strings : num_proc;

  for (i = 0; i < maxiterations; i++)
  {
    // Do stuff
    if ((childpid = fork()) == -1)
    {
      perror("Failed to create child process.");
      if (detachandremove(id, sharedstrings) == -1)
        perror("Failed to detach and remove shared memory segment.");
    }

    child_cnt++; // Add one to currently running children count

    // Child Code
    if (childpid == 0)
    {
      // TODO: - Replace hardcoded index to test, will need to use counter and replace in loop

      char strid[100+1] = {'\0'};
      sprintf(strid, "%d", id);

      char strrow[100+1] = {'\0'};
      sprintf(strrow, "%d", i);

      char strmaxsize[100+1] = {'\0'};
      sprintf(strmaxsize, "%d", STR_SIZE);

      char *args[5] = {"palin", strid, strrow, strmaxsize, '\0'};
      /* Arguments sent to exec-ed 'palin'
        * argv[0] = "palin" executable
        * argv[1] = id of shared memory array
        * argv[2] = (index of string in shared memory array to test i.e. The 'row' in array to start from)
        * argv[3] = max size of strings
      */
      execv(args[0], args);
      perror("Child failed to exec command!\n");
    }

    // Parent code
    // TODO: - set timer to -t/run_time value, if it runs out kill all children, print message, exit
 
    // If num of children running is at max wait before next loop
    if (child_cnt == num_children)
    {
      wpid = wait(NULL);
      printf("Parent waited for pid: %i", wpid);
    }
  }



  // Detach from and remove the shared memory array segment
  detachandremove(id, sharedstrings);
  
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
  printf("./master -n 10 -s 4 -t 60\n\n");
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
}
