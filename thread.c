#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>

#include "thread.h"

/**************************************************************/
/* Purpose: Thread function to query to the PPS for Pokemon of a specific type and update the search_results array.

Input:
- arg: Pointer to a client_socket allowing for communication with a tcp/ip server to query and get the pokemon data,
for those with a particular type.

Output:
- arg: Pointer to a struct SearchParams containing search parameters, result storage,
and other data neccesary for thread functionality.

   - arg.search_results: Pointer from struct SearchParams to pointer to an array of struct Pokemon
     pokemon found in search results are saved to it

   - args.search_results_amount: Pointer from struct SeachParams to an int pointer that gets updated
     whenever a pokemon matching the search results are found.

   - args.mutex: Pointer from struct SeachParams to a pointer to a pthread_mutex_t which controls locking
     and unlocking of data flow between the program ensuring multiple threads, or the main don't override
     the same data and corrupt it in memory.

Return:
None
*/
/*************************************************************/
void* typeSearch(void* arg) {
  struct SearchParams* params = (struct SearchParams*) arg; // Cast to right type
  ssize_t bytes_sent = send(params->client_socket, params->type_to_search, sizeof(params->type_to_search), 0);
  if (bytes_sent == -1) {
    return NULL;
  }

  // Receive serialized data from the server
  uint8_t serialized_data[75000];  // Adjust buffer size as needed
  ssize_t bytes_received = recv(params->client_socket, serialized_data, sizeof(serialized_data), 0);
  
  if (bytes_received == -1) {
    return NULL;
  }

  // Deserialize and process the received data
  struct Pokemon *search_results;
  int search_result_count;

  if (deserializePokemonArray(serialized_data, bytes_received, &search_results, &search_result_count) == -1)
    return NULL;

  for (int i = 0; i < search_result_count; i++) {
    if (*params->terminate_thread == 1) return NULL;

    *params->search_results = (struct Pokemon*) realloc(*params->search_results, sizeof(struct Pokemon) * (*params->searches_results_amount + 1));

    if (*(params->search_results) == NULL) {
      free(search_results);
      return NULL;
    }

    pthread_mutex_lock(params->mutex);
    (*params->search_results)[*(params->searches_results_amount)] = search_results[i];
    (*params->searches_results_amount)++;
    pthread_mutex_unlock(params->mutex);
  }

  // Increment the number of successful queries made during this session
  pthread_mutex_lock(params->mutex);
  (*params->queries_successfully_made)++;
  pthread_mutex_unlock(params->mutex);
  free(search_results);
  
  return NULL;
}

/**************************************************************/
/* Purpose: Thread function to save search results to a file.

Input:
- arg: Pointer to a struct SaveParams containing file information and search results.


Output:
- arg: Pointer to a struct SaveParams containing search parameters, result storage,
and other data neccesary for thread functionality.

   - arg.files_made_arr: Pointer from struct SaveParams to pointer to an array of character pointers,
     if the file and saved successfully it is saved to the array

   - args.files_made_amount: Pointer from struct SaveParans to an int pointer that gets updated
     whenever a file is successfully made.

   - args.mutex: Pointer from struct SaveParams to a pointer to a pthread_mutex_t which controls locking
     and unlocking of data flow between the program ensuring multiple threads, or the main don't override
     the same data and corrupt it in memory.

Return:
None

Note: The filenames and search results are stored in dynamically allocated arrays, and the memory is managed in the main thread.
*/
/**************************************************************/
void* saveResults(void* arg) {
  struct SaveParams* params = (struct SaveParams*)arg;

  for (int i = 0; i < params->searches_results_amount; i++) {
    if (*params->terminate_thread == 1) return NULL;

    pthread_mutex_lock(params->mutex);

    // Write each Pokemon's data to the file
    fprintf(params->file, "%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
            params->search_results[i].id, params->search_results[i].name, params->search_results[i].type1,
            params->search_results[i].type2, params->search_results[i].total, params->search_results[i].hp,
            params->search_results[i].attack, params->search_results[i].defense, params->search_results[i].sp_attack,
            params->search_results[i].sp_def, params->search_results[i].speed, params->search_results[i].generation,
            params->search_results[i].legendary);
    
    pthread_mutex_unlock(params->mutex);
  }

   // Add the successfully made file to the files_made_arr array
  *params->files_made_arr = (char **) realloc(*params->files_made_arr, sizeof(char*) * (*params->files_made_amount + 1));
  if (*params->files_made_arr == NULL) return NULL;

  // Increment the number of successful files made during this session
  (*params->files_made_arr)[*(params->files_made_amount)] = params->filename;
  (*params->files_made_amount)++;

  // Close the file and return
  fclose(params->file);
  return NULL;
}

/**************************************************************/
/* Purpose: Thread function to handle client communication.

Input:
- arg: Pointer to a ClientParams struct containing client information and Pokemon data.

Output:
None

Return:
None

*/
/**************************************************************/
void *handleClient(void *arg) {
  struct ClientParams* params = (struct ClientParams*) arg;
  int client_socket = params->client_socket;

  char buffer[1024];
  ssize_t bytes_received;

  while (1) { // Going into infinite loop to talk to client
    // Getting message from the client
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received == -1) {
      printf("SERVER: Error receiving data from client.\n");
      return NULL;
    }

    if (bytes_received == 0) {
      printf("SERVER: Client disconnected.\n");
      return NULL;
    }

    buffer[bytes_received] = '\0'; // Null-terminating the received data to treat it as a string

    struct Pokemon *search_results = NULL;
    int search_result_count = 0;

    // Searching for Pokemon of the specified type
    for (int i = 0; i < params->pokemon_count; i++) {
      if (strcmp(params->pokemon_array[i].type1, buffer) == 0) {
        search_results = (struct Pokemon*) realloc(search_results, sizeof(struct Pokemon) * (search_result_count + 1));
        search_results[search_result_count] = params->pokemon_array[i];
        search_result_count++;
      }
    }

    uint8_t *serialized_data;
    size_t serialized_size;

    // Serializing the search results for transmission and sending them
    serializePokemonArray(search_results, search_result_count, &serialized_data, &serialized_size);
    ssize_t bytes_sent = send(client_socket, serialized_data, serialized_size, 0);

    if (bytes_sent < 0) {
      printf("SERVER: Error sending data to client.\n");
      return NULL;
    }
  }

  // Closing the client socket when done
  close(client_socket);
  return NULL;
}