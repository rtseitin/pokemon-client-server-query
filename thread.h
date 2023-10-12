#include <pthread.h>
#include <stdint.h>
#include "utils.h"

struct SearchParams { // Define a structure to hold search parameters for type search
  int client_socket;
  char type_to_search[TYPE_STRING_LENGTH];
  struct Pokemon **search_results;
  int *searches_results_amount;
  volatile int *terminate_thread;
  pthread_mutex_t *mutex;
  int *queries_successfully_made;
};

struct SaveParams { // Define a structure to hold save parameters for saving search results
  FILE *file;
  struct Pokemon *search_results;
  int searches_results_amount;
  volatile int *terminate_thread;
  pthread_mutex_t *mutex;
  char ***files_made_arr;
  int *files_made_amount;
  char *filename;
};

struct ClientParams {
  struct Pokemon *pokemon_array;
  int client_socket;
  int pokemon_count;
};

void* typeSearch(void* arg);
void* saveResults(void* arg);
void *handleClient(void *arg);