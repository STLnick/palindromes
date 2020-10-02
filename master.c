#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
  int num_proc, num_children, opt, proc_cnt, run_time;
  int hflag, nflag, sflag, tflag; // Flags for command line options
  hflag = nflag = sflag = tflag = 0;
  proc_cnt = 0; // Counter for currently running children processes

  // Parse command line options
  while ((opt = getopt(argc, argv, "hn:s:t:")) != -1)
  {
    switch (opt)
    {
      case 'h': // Help - describe how to run program and default values
        hflag = 1;
        printf("-h option used. describe and TERMINATE.");
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
    }
  }

  // Set default values for unused options if -h wasn't used
  if (!hflag)
  {
    num_proc = nflag ? num_proc : 4;
    num_children = sflag ? num_children : 2;
    run_time = tflag ? run_time : 100;
  }

  printf("num_proc: %d\n", num_proc);
  printf("num_children: %d\n", num_children);
  printf("run_time: %d\n", run_time);
  return 0;
}
