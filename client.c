#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "thread.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7001

int main() {
  int client_socket;
  struct sockaddr_in server_address;
  
  // Creating the client socket
  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (client_socket < 0) {
    printf("*** CLIENT ERROR: Could not open socket.\n");
    return -1;
  }

  // setting up address
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
  server_address.sin_port = htons((unsigned short) SERVER_PORT);

  int status;
  
  // Connecting to server
  status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  if (status < 0) {
    printf("Unable to establish connection to the PPS!\n");
    return -1;
  }

  // Variables for menu choice and thread management
  char choice = 0;
  pthread_t *threads = NULL;
  pthread_t thread;
  int thread_count = 0;
  struct Pokemon *search_results = NULL;
  int searches_results_amount = 0;
  volatile int terminate_thread = 0;
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  int queries_successfully_made = 0;
  char **files_made_arr = NULL;
  int files_made_amount = 0;

  // Main loop to display the menu and perform user-selected actions
  while (choice != 'c') {
    printf("\nMenu:\n");
    printf("a. Type Search\n");
    printf("b. Save Results\n");
    printf("c. Exit the program\n");
    printf("Enter your choice: ");
    
    scanf(" %c", &choice);
    while (getchar() != '\n');

    switch (choice) {
      case 'a': // Perform type searc
        printf("\nEnter the type to search for: ");
        char type_to_search[TYPE_STRING_LENGTH];
        scanf("%14s", type_to_search);

        // Set search parameters for the type search thread
        struct SearchParams params = {
          .client_socket = client_socket,
          .search_results = &search_results,
          .searches_results_amount = &searches_results_amount,
          .terminate_thread = &terminate_thread,
          .mutex = &mutex,
          .queries_successfully_made = &queries_successfully_made
        };
        strcpy(params.type_to_search, type_to_search);


        // Allocate memory for the thread array and create a new thread for type search
        threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * thread_count+1);

        if (threads == NULL) {
          printf("Failed to allocate memory.\n");

          // Graceful ternination
          terminate_thread = 1;
          pthread_mutex_destroy(&mutex);
          for (int i=0; i<files_made_amount; i++) free(files_made_arr[i]);
          free(files_made_arr);
          free(search_results);
          free(threads);
        }

        thread = threads[thread_count];
        thread_count++;

        if (pthread_create(&thread, NULL, typeSearch, (void*)&params) != 0) {
          printf("Error creating search thread.\n");

          // Graceful ternination
          terminate_thread = 1;
          pthread_mutex_destroy(&mutex);
          for (int i=0; i<files_made_amount; i++) free(files_made_arr[i]);
          free(files_made_arr);
          free(search_results);
          free(threads);
          return 1;
        }

        pthread_detach(thread);

        break;

      case 'b': // Save search results to a file
        if (searches_results_amount == 0) {
          printf("\nNo search results to save.\n");
          break;
        }

        FILE* file = NULL;
        char *filename = NULL;

        // Get the filename to save search results and check if the file can be opened
        printf("\n");
        
        int status = getFile(&file, &filename, "w", "Unable to create the new file. Please enter the name of the file again.");
        if (status < 0) {
          // Graceful ternination
          terminate_thread = 1;
          pthread_mutex_destroy(&mutex);
          for (int i=0; i<files_made_amount; i++) free(files_made_arr[i]);
          free(files_made_arr);
          free(search_results);
          free(threads);
          return 1;
        } else if (status == 1) continue;

        // Set save parameters for the save thread
        struct SaveParams save_params = {
          .file = file,
          .search_results = search_results,
          .searches_results_amount =searches_results_amount,
          .terminate_thread = &terminate_thread,
          .mutex = &mutex,
          .files_made_arr = &files_made_arr,
          .files_made_amount = &files_made_amount,
          .filename = filename
        };

        // Allocate memory for the thread array and create a new thread for saving
        threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * thread_count+1);

        if (threads == NULL) {
          printf("Failed to allocate memory.\n");

          // Graceful ternination
          terminate_thread = 1;
          pthread_mutex_destroy(&mutex);
          for (int i=0; i<files_made_amount; i++) free(files_made_arr[i]);
          free(files_made_arr);
          free(search_results);
          free(threads);
          return 1;
        }

        thread = threads[thread_count];
        thread_count++;

        if (pthread_create(&thread, NULL, saveResults, (void*)&save_params) != 0) {
          printf("Error creating save thread.\n");

          // Graceful ternination
          terminate_thread = 1;
          pthread_mutex_destroy(&mutex);
          for (int i=0; i<files_made_amount; i++) free(files_made_arr[i]);
          free(files_made_arr);
          free(search_results);
          free(threads);
          return 1;
        }

        pthread_detach(thread);

        break;

      case 'c': // Exit the program
        // Graceful termination
        terminate_thread = 1;
        pthread_mutex_destroy(&mutex);

        // Display the number of successful queries and the files made during the session
        printf("\n# of succesfull queries completed during session: %d\n", queries_successfully_made);
        printf("Files made during session: ");

        // Display the filenames of the files created during the session
        if (files_made_amount == 0) printf("N/A\n");
        else {
          for (int i = 0; i < files_made_amount-1; i++) {
            printf("%s, ", files_made_arr[i]);
            free(files_made_arr[i]);
          }
          printf("%s\n", files_made_arr[files_made_amount-1]);
          free(files_made_arr[files_made_amount-1]);
        }

        // Free allocated memory
        free(files_made_arr);
        free(search_results);
        free(threads);

        close(client_socket);

        break;

      default: // Invalid choice
        printf("\nInvalid choice. Please try again.\n");
        break;
    }
  }

  return 0;
}