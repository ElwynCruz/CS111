#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
struct termios save;
const char crlf[] = {0x0D, 0x0A};
int shell_flag = 0;
int pid;
// two pipes
int ptoc[2];    // parent to child
int ctop[2];    // child to parent


void restore_original() {
  tcsetattr(0, TCSANOW, &save);
  if (shell_flag) {
    int stat = 0;
    waitpid(pid, &stat, 0);
    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\n",  WTERMSIG(stat), WEXITSTATUS(stat)); 
  }
}

void handler(int signum) {
  if (shell_flag && signum == SIGINT) {
    restore_original();
    exit(0);
  }
  if (signum == SIGPIPE) {
    restore_original();
    exit(0);
  }
}

void read_write(int fdin, int fdout) {
  char buffer[256]; // arbitrary buffer size
  int i = 0;
  while (1) {
    i = read(fdin, buffer, 256);    
    int j;
    for (j = 0; j < i; j++) {
      if (buffer[j] == 0x04) { // EOF/^D
	if (shell_flag) {
	  break;
	}
	else {
	  restore_original();
	  exit(0);	  
	}
      }
      else if(buffer[j] == 0x0A || buffer[j] == 0x0D) {   // new line/carriage return
	write(fdout, crlf, 2); // write out cr and then lf
      }
      else {   // write out the character
	write (fdout, buffer+j, 1); 
      }
    }
  }
}


int main(int argc, char* argv[]){

  
  tcgetattr(0, &save);
  struct termios changes;
  tcgetattr(0, &changes);
  //changes from spec
  changes.c_iflag = ISTRIP;  /* only lower 7 bits*/
  changes.c_oflag = 0;       /* no processing*/
  changes.c_lflag = 0;       /* no processing*/
  tcsetattr(0, TCSANOW, &changes);
  
  int c;

  static struct option long_options[] = {
    { "shell", required_argument, 0, 's'},
    {0,        0,                 0,  0 }
  };
  
  char *shell;  

  while ((c = getopt_long(argc, argv, "s:", long_options, NULL)) != -1) {
    switch (c) {
    case 's':
      shell_flag = 1;
      shell = optarg;
      signal(SIGINT, handler);
      signal(SIGPIPE, handler);
      break;
    default: // incorrect arg
      fprintf(stderr, "Usage: /lab1a --shell=[executable]\n");
      exit(1);
    }
  }

  if (shell_flag) {
    //pipe

    if (pipe(ptoc) == -1) {
      //error
      fprintf(stderr, "pipe1 failed \n");
      exit(1);
    }
    if (pipe(ctop) == -1) {
      fprintf(stderr, "pipe2 failed \n");
      exit(1);
    }
    
      
    

    pid = fork();

    
    if (pid == -1) {
      //error
      fprintf(stderr, "error forking");
      exit(1);
    }
    else if (pid == 0) { // child
      close(ptoc[1]);
      close(ctop[0]);
      dup2(ptoc[0], 0);
      dup2(ctop[1], 1);
      dup2(ctop[1], 2);
      close(ptoc[0]);
      close(ctop[1]);
      execl( shell, "sh", (char *) NULL );
    }
    else {     // parent
      close(ptoc[0]);
      close(ctop[1]);
      

      
      struct pollfd inputs[] = {
	{0      , POLLIN, 0},   // stdin to terminal
	{ctop[0], POLLIN, 0}    // output from shell to terminal
      };
      

      while (1) {
	if (poll(inputs, 2, 0) > 0) {
	  if (inputs[0].revents == POLLIN) {  // data to read from stdin
	    char buffer[256]; // arbitrary buffer size
	    int i = 0;
	    i = read(0, buffer, 256);    
	    int j;
	    for (j = 0; j < i; j++) {
	      
	      if (buffer[j] == 0x0D || buffer[j] == 0x0A) {
		write(ptoc[1], crlf+1, 1); // goes to shell as just lf
		write(1,crlf, 2);
	      }
	      else if (buffer[j] == 0x04) {    // ^D
		char controlD[] = {"^D"};
		write (1, controlD, 2);
		write(1,crlf, 2);
		close(ptoc[1]);
	      }
	      else if (buffer[j] == 0x03) {    // ^C
		char controlC[] = {"^C"};
		write(1, controlC, 2);
		write(1,crlf, 2);
		kill(pid, SIGINT);
	      }
	      else {
		write(1, buffer+j, 1); 
		write(ptoc[1], buffer+j,1);
	      } 
	    }
	  }
	  else if (inputs[0].revents == POLLERR) {
	    fprintf(stderr, "Poll error with stdin\n");
	    exit(1);
	  }
	  if (inputs[1].revents == POLLIN) {
	    char buffer[256]; // arbitrary buffer size
	    int i = 0;
	    i = read(ctop[0], buffer, 256);  
	    int j;
	    for (j = 0; j < i; j++) {
	      if (buffer[j] == 0x0D || buffer[j] == 0x0A) { // CRLF
		write(1,crlf, 2);
	      }	
	      else {
		write (1, buffer+j, 1);
	      }
	    }
	  }
	  else if (inputs[1].revents == POLLERR || inputs[1].revents & POLLHUP) {
	    close(ctop[0]);
	    
	    
	    restore_original();
	    exit(0);
	  }
	}
      }	
    }
  
  }
  else {
    read_write(0,1);
  }
  
  return 0;
}
