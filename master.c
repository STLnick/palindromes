#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int detachandremove(int shmid, void *shmaddr);

int main (int argc, char **argv)
{
  int childpid; // PID of a child process
  int id; // ID for the shared memory segment
  int key = 93; // Key to allocate shared memory segment
  int num_proc, num_children, opt, proc_cnt, run_time;
  int nflag = 0, sflag = 0, tflag = 0; // Flags for command line options
  proc_cnt = 0; // Counter for currently running children processes

  // Parse command line options
  while ((opt = getopt(argc, argv, "hn:s:t:")) != -1)
  {
    switch (opt)
    {
      case 'h': // Help - describe how to run program and default values
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

  printf("num_proc: %d\n", num_proc);
  printf("num_children: %d\n", num_children);
  printf("run_time: %d\n", run_time);


  // TESTING STRING ARRAY
  char *strings[3] = {"how's", "it", "going?"};
  char *sharedstrings[3];

  // dummy shared value
  int *sharedtotal;

  // Allocate shared memory segment
  // TODO: Change the sizeof() after testing with simple data type
  if ((id = shmget(key, sizeof(int), IPC_CREAT | 0660)) == -1)
  {
    perror("Failed to create shared memory segment.");
    return 1;
  }

  // TESTING MESSAGE IF WE SUCCESSFULLY GET SHARED MEMORY
  printf("Successfully allocated shared memory with id: %d\n", id);

  // Attach to the allocated shared memory
  //if ((sharedstrings = (char *) shmat(id, NULL, 0)) == (void *) - 1)
  //{
  //  perror("Failed to attach shared memory segment.");
  //  if (shmctl(id, IPC_RMID, NULL) == -1)
  //    perror("Failed to remove memory segment.");
  //  return 1;
  //}

  // TESTING WITH BASIC DATA TYPE Attach to the allocated shared memory
  if ((sharedtotal = (int *) shmat(id, NULL, 0)) == (void *) - 1)
  {
    perror("Failed to attach shared memory segment.");
    if (shmctl(id, IPC_RMID, NULL) == -1)
      perror("Failed to remove memory segment.");
    return 1;
  }

  // Detach from and remove the shared memory segment
  // TODO: TESTING WITH SIMPLE DATA TYPE: Change second argument after testing
  detachandremove(id, sharedtotal);



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
