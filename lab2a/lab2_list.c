
//NAME: Elwyn Cruz
//EMAIL: ElwynCruz@g.ucla.edu
//ID: 104977892

#include <pthread.h>
#include "SortedList.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define BILLION 1e9

int numThreads = 1;
int numIterations = 1;
int opt_yield = 0;
char test[15];
char* yield_ops = "none";
SortedList_t *head = NULL;
SortedListElement_t *elements = NULL;
char syncType = 'n';

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int lock = 0; 


char* rand_key() {
  srand((unsigned int)time(NULL));
  int length = 5;
  char* key = malloc(length+1); //+1 for null char
  int i;
  for (i = 0; i < length; i++) {
    char randChar = (rand() % 26) + 65;
    int isLower = rand() % 2;
    if (isLower) {
      randChar = tolower(randChar);
    }
    key[i] = randChar;
  }
  key[length] = '\0';
  return key;

}



void handler(int signum) {
  if (signum == SIGSEGV) {
    fprintf( stderr, "Segmentation fault caught. Error code: 2. Exiting now...\n");
    exit(2);
  }
}
// do thread stuff to list
// arg - position of element array thread should start
void * thread_list(void* arg) { 
  int index = *((int*) arg);
  int i;
  
  switch(syncType) {
  case 'm':
    pthread_mutex_lock(&mutex);
    break;
  case 's':
    while(__sync_lock_test_and_set(&lock, 1));
    break;
  }
  
  //insert
  for(i=index; i < index+numIterations; i++) {
    SortedList_insert(head, &elements[i]);
  }

  switch (syncType) {
  case 'm':
    pthread_mutex_unlock(&mutex);
    break;
  case 's':
    __sync_lock_release(&lock);
    break;
  }

  //get length

  switch(syncType) {
  case 'm':
    pthread_mutex_lock(&mutex);
    break;
  case 's':
    while(__sync_lock_test_and_set(&lock, 1));
    break;
  }

  int length;
  length = SortedList_length(head);
  if (length < 0) {
    fprintf(stderr, "Error: length < 0, list corrupted\n");
    exit(2);
  }

  switch (syncType) {
  case 'm':
    pthread_mutex_unlock(&mutex);
    break;
  case 's':
    __sync_lock_release(&lock);
    break;
  }

  // delete each key we previously inserted

  switch(syncType) {
  case 'm':
    pthread_mutex_lock(&mutex);
    break;
  case 's':
    while(__sync_lock_test_and_set(&lock, 1));
    break;
  }
  
  for (i=index; i<index+numIterations; i++) {
    SortedListElement_t *curr = SortedList_lookup(head, elements[i].key);
    if (curr == NULL) {
      fprintf(stderr, "Error: lookup failed\n");
      exit(2);
    }
    if (SortedList_delete(curr)) {
      fprintf(stderr, "Error: delete failed\n");
      exit(2);
      }
  }  

  switch (syncType) {
  case 'm':
    pthread_mutex_unlock(&mutex);
    break;
  case 's':
    __sync_lock_release(&lock);
    break;
  }

  return NULL;
}

void getTest() {
  char* base = "list-";
  strcpy(test, base);
  strcat(test, yield_ops);
  strcat(test, "-");
  switch (syncType) {
  case 'm':
  case 's':
    strcat(test, &syncType);
    break;
  default:
    strcat(test, "none");
    break;
  }
}

int main(int argc, char *argv[]) {

  static struct option long_options[] = {
    {"threads",    required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield",      required_argument, 0, 'y'},
    {"sync",       required_argument, 0, 's'},
    {  0   ,       0,                 0,  0 }
  };

  signal(SIGSEGV, handler); //watch for seg faults
  
  int c; 
  while ( (c = getopt_long(argc, argv, "t:i:y:s:", long_options, NULL)) != -1) {
    switch (c) {
    case 't':
      numThreads = atoi(optarg);
      break;
      
    case 'i':
      numIterations = atoi(optarg);
      break;
    case 'y':
      if (strlen(optarg) > 3) {
	fprintf( stderr, "Error: too many arguments to yield. Valid: --yield=[idl]\n");
	exit(1);
      }
      unsigned int i;
      for (i = 0; i < strlen(optarg); i++) {
	yield_ops = optarg;
	switch (optarg[i]) {
	case 'i':
	  opt_yield |= INSERT_YIELD;
	  break;
	case 'd':
	  opt_yield |= DELETE_YIELD;
	  break;
	case 'l':
	  opt_yield |= LOOKUP_YIELD;
	  break;
	default:
	  fprintf(stderr, "Error: Bad argument to yield. Valid: --yield=[idl]\n");
	  exit(1);
	  break;
	}
      }
      break;
    case 's':
      if (strlen(optarg) > 1) {
	fprintf(stderr, "Error: too many arguments to sync\n");
      }
      syncType = optarg[0];
      break;
    default:
      fprintf(stderr, "Error: invalid argument. Usage: ./lab2_list --thread=# --iterations=# --yield=[idl] --sync=[s|m]\n");
      exit(1);
    }
  }
  head = malloc(sizeof(SortedList_t));
  head->prev = head;
  head->next = head;
  head->key = NULL;
 
  // init elements w random keys
  
  int numElements = numThreads * numIterations;
  elements = malloc(numElements * sizeof(SortedListElement_t));
  int i;
  for(i = 0; i < numElements; i++) {
    elements[i].key = rand_key();
  }
  
  // make threads
  pthread_t* threads = (pthread_t*) malloc(numThreads*sizeof(pthread_t));
  if (threads == NULL) {
    fprintf(stderr, "Error: Could not allocate memory for threads\n");
    free(elements);
    free(head);
    exit(1);
  }
  
  int thread_args[numThreads];
  for (i=0; i<numThreads;i++) {
    thread_args[i] = i * numIterations;
  }

  // get start time
  struct timespec start, end;
  if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
    fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
    exit(1);
  }
  //create threads
  

  for(i=0; i < numThreads; i++) {

    if (pthread_create(&threads[i], NULL, thread_list, &thread_args[i]) != 0) {
      fprintf(stderr, "Error: pthread_create failed\n");
      free(threads);
      free(elements);
      free(head);
      exit(1);
    }
  }
  // join threads
  for(i=0; i < numThreads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error pthread_join failed\n");
      free(threads);
      free(elements);
      free(head);
      exit(1);
    }
  }

  // get end time
  if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
    fprintf(stderr, "Error: Failure in clock_gettime(). Exit with 1.\n");
    free(threads);
    free(elements);
    free(head);
    exit(1);
  }
  
  long long total_time = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  long long operations = numThreads * numIterations * 3;
  long long average_time =  total_time / operations;

  // getTest name
  getTest();
  printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test, numThreads, numIterations, 1, operations, total_time, average_time);
  
  free(threads);
  free(elements);
  free(head);
  exit(0);
  
}




