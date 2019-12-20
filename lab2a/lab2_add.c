//NAME: Elwyn Cruz
//Email: ElwynCruz@g.ucla.edu
//ID: 104977892
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <sched.h>
#include <string.h>

#define BILLION 1E9

static long long counter = 0;
int numIterations = 1;
int numThreads = 1;
int opt_yield = 0;
char syncType = 'n'; // default none
char test[15]; // all tests are an add test

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int lock = 0; // unlocked
int sharedCounter = 0; // unlocked

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}


void add_m() {
  int i;
  for (i=0; i< numIterations; i++) {
    pthread_mutex_lock(&mutex);
    add(&counter, 1);
    pthread_mutex_unlock(&mutex);
  }
  for (i=0; i< numIterations; i++) {
    pthread_mutex_lock(&mutex);
    add(&counter, -1);
    pthread_mutex_unlock(&mutex);
  }
}

void add_s() {
  int i;
  for (i=0; i< numIterations; i++) {
    while (__sync_lock_test_and_set(&lock,1) == 1)
      ;
    add(&counter, 1);
    __sync_lock_release(&lock);
  }
  for (i=0; i< numIterations; i++) {
    while (__sync_lock_test_and_set(&lock,1) == 1)
      ;
    add(&counter, -1);
    __sync_lock_release(&lock);
  }
}

void add_c() {
  long long cur;
  long long next;
  int i;
  for (i = 0; i < numIterations; i++) {
    do {
      if (opt_yield) {
	sched_yield();
      }
      cur = counter;
      next = cur + 1;
    } while (__sync_val_compare_and_swap(&counter, cur, next) != cur);
  }
  
  for (i = 0; i < numIterations; i++) {
    do {
      if (opt_yield) {
	sched_yield();
      }
      cur = counter;
      next = cur - 1;
    } while (__sync_val_compare_and_swap(&counter, cur, next) != cur);
  }
}

void thread_add() {
  int i;

  switch (syncType) {
  case 'n':
    for (i=0; i< numIterations; i++) {
      add(&counter, 1);
    }
    for (i=0; i< numIterations; i++) {
      add(&counter, -1);
    }
    break;
  case 'm':
    add_m();
    break;
  case 's':
    add_s();
    break;
  case 'c':
    add_c();
    break;
  default:
    fprintf(stderr, "Error: Invalid test\n");
    exit(1);
  }
}


void getTest() {
  char *add = "add-";
  strcpy(test, add);
  if (opt_yield) {
    strcat(test, "yield-");
  }
  switch (syncType) {
  case 'm':
  case 's':
  case 'c':
    strcat(test, &syncType);
    break;
  case 'n':
    strcat(test, "none");
    break;
  default:
    fprintf(stderr, "Unrecognized test\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
 
  int c;
  static struct option long_options[] = {
    { "threads"   , required_argument, 0, 't'},
    { "iterations", required_argument, 0, 'i'},
    { "yield"     , no_argument      , 0, 'y'},
    { "sync"      , required_argument, 0, 's'},
    { 0           , 0                , 0,  0 }
  };
  while ( (c = getopt_long(argc, argv, "t:i:y", long_options, NULL)) != -1) {
    switch (c) {
    case 't':
      numThreads = atoi(optarg);
      break;
    case 'i':
      numIterations = atoi(optarg);
      break;
    case 'y':
      opt_yield = 1;
      break;
    case 's':

      syncType = optarg[0];
      break;
    default:
      fprintf(stderr, "Usage: ./lab2a_add threads=# iterations=#\n");
      exit(1);
    }
  }
  // figure out which test we're running
  getTest();

  
  // allocate space for threads
  pthread_t *threads = (pthread_t*) malloc(numThreads * sizeof(pthread_t));
  if (threads == NULL) {
    fprintf(stderr, "Error could not allocate space for threads\n");
    exit(1);
  }
  // get start time
  struct timespec start, end;
  if (clock_gettime(CLOCK_REALTIME, &start) < 0) {
    fprintf(stderr, "Error getting time\n");
    exit(1);
  }
  
  // make threads and do work
  int i;
  for(i=0; i < numThreads;i++) {
    if(pthread_create(&threads[i], NULL, (void*) thread_add, NULL) != 0) {
      fprintf(stderr, "Error creating thread\n");
      free(threads);
      exit(1);
    }
  }
  // join threads
  for(i=0; i < numThreads;i++) {
    if (pthread_join(threads[i], NULL) < 0) {
      fprintf(stderr, "Error joining thread\n");
      exit(1);
    }
  }
  // get end time
  if (clock_gettime(CLOCK_REALTIME, &end) < 0) {
    fprintf(stderr, "Error getting time\n");
    exit(1);
  }

  // calculate run time in nanoseconds
  long long total_time = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec -start.tv_nsec);
  // calculate average time per operation
  long long operations = numThreads * numIterations * 2;
  long long average_time = total_time / operations; // each thread runs n iterations, each iteration has 2 operations
  
  // print out CSV record
  printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", test, numThreads, numIterations, operations, total_time, average_time, counter);
  free(threads);
  exit(0);
  return 0;
}
