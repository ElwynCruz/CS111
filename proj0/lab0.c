/*
NAME: Elwyn Cruz
EMAIL: ElwynCruz@g.ucla.edu
ID: 104977892
 */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
void makeFault() {
  char *ch;
  ch = NULL;
  *ch = 'a';
}

void handler(int sigNum) {
  if (sigNum == SIGSEGV)
    {
      fprintf(stderr, "Segmentation fault caught, signal number: %d \n", sigNum);
      exit(4);
    }
}

int main(int argc, char *argv[])
{

  int c;
  int segFlag;
  char* inf = NULL;
  char* outf = NULL;
  int fd;
  static struct option long_options[] = {
    {"input",    required_argument, 0,  'i' },
    {"output",   required_argument, 0,  'o' },
    {"segfault", no_argument,       0,  's' },
    {"catch",    no_argument,       0,  'c' },
    {0,          0,                 0,   0  }
  };
  while ( (c = getopt_long(argc, argv, "i:o:sc", long_options, NULL)) != -1) {
  
  
    switch (c)
      {
      case 'i':    // input
	inf = optarg;
	if (inf) {
	  fd = open(inf, O_RDONLY);
	  if (fd >= 0) {
	    close(0);
	    dup(fd);
	    close(fd);
	  }
	  else {
	    fprintf(stderr, "Error: Could not open input file %s. Reason: %s\n" ,optarg, strerror(errno));
	    exit(2);
	  }
	}
	break;
      case 'o':    // output
	outf = optarg;
	if (outf) {
	  fd = creat(optarg, 0666);
	  if (fd >= 0) {
	    close(1);
	    dup(fd);
	    close(fd);
	  }
	  else { 
	    fprintf(stderr, "Error: Could not write to file %s. Reason: %s\n" ,optarg, strerror(errno));
	    exit(3);
	  }
	} 
	  
	break;
      case 's':    // segfault
	segFlag = 1;
	break;
      case 'c':    // catch
	signal(SIGSEGV, handler);
	break;
      default: // invalid 
	fprintf(stderr, "Error: invalid argument %s", argv[optind-1]);
	fprintf(stderr, "Usage: lab0 --input=[file] --output=[file] --segfault --catch\n");
	exit(1);
      }
  }
  char ch;
  if (segFlag)
    makeFault();
  while (read(0, &ch, 1) > 0)   // read character by character
    {
      write(1, &ch, 1);
    }
  exit(0);
}

