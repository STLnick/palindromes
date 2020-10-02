#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
  int num_proc, num_children, opt, proc_cnt, run_time;
  int nflag, sflag, tflag; // Flags for command line options
  nflag = sflag = tflag = 0;
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
  return 0;
}
