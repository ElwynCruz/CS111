#include <stddef.h>
#include <unistd.h>


int main (int argc, char *argv[]) {
  execl( "/bin/sh", "sh", (char*) NULL );
}
