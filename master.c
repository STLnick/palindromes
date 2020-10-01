#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
  int num_proc, num_children, opt, proc_cnt, run_time;
  int hflag, nflag, sflag, tflag; // Flags for command line options
  hflag = nflag = sflag = tflag = 0;
  proc_cnt = 0; // Counter for currently running children processes

  while ((opt = getopt(argc, argv, "hn:s:t:")) != -1)
  {
    switch (opt)
    {
      case 'h': // The default values case
        hflag = 1;
        num_proc = 4; // Number of processes to be ran in total
        num_children = 2; // Number of children to be running at any one time
        run_time = 100; // Seconds the program will run if processes don't finish first
        break; 
      case 'n':
        nflag = 1;
        num_proc = atoi(optarg);
        break;
      case 's':
        sflag = 1;
        num_children = atoi(optarg);
        break;
      case 't':
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
