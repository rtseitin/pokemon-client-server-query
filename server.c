#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "thread.h"

#define SERVER_PORT 7001

int main() {
  int status;

  // Declaration of variables and initialization
  FILE *file = NULL;
  char *filename = NULL;

  // Get the filename from the user and check if the file exists or user exits
  printf("\n");
  status = getFile(&file, &filename, "r", "Pokemon file is not found. Please enter the name of the file again.");
  if (status < 0) {
    free(filename);
    return -1;
  } else if (status == 1) {
    free(filename);
    return 0;
  }

  free(filename);

  // Read Pokémon data from the file and store it in pokemon_array
  struct Pokemon *pokemon_array = NULL;
  int pokemon_count = 0;

  if (readPokemonData(file, &pokemon_array, &pokemon_count) == 1) return -1;
  fclose(file);

  int server_socket;
  struct sockaddr_in server_address;

  // Creating the server socket
  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    printf("*** SERVER ERROR: Could not open socket!\n");
    return -1;
  }

  // Setting up the server address
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons((unsigned short) SERVER_PORT);

  // Binding the server socket
  status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
  if (status < 0) {
    printf("*** SERVER ERROR: Could not bind socket!\n");
    return -1;
  }

  status = listen(server_socket, 5); // Setting up the line-up to handle up to 5 clients
  if (status < 0) {
     printf("*** SERVER ERROR: Could not listen on socket!\n");
     return -1;
  }

  printf("\nSERVER: Awaiting connections from clients.\n");

  pthread_t *threads = NULL;
  pthread_t thread;
  int thread_count = 0;

  while (1) { // Waiting for clients

    struct sockaddr_in client_address;
    int client_socket;

    socklen_t address_size = sizeof(client_address);
  
    // Accepting incoming client connection
    client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_size);
    if (client_socket < 0) {
      printf("*** SERVER ERROR: Could accept incoming client connection.\n");
      return -1;
    }

    printf("SERVER: Received client connection.\n");

    threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * thread_count+1);

    if (threads == NULL) {
      printf("SERVER: Failed to allocate memory.\n");
      free(threads);
      return -1;
    }

    struct ClientParams params = {
      .client_socket = client_socket,
      .pokemon_array = pokemon_array,
      .pokemon_count = pokemon_count
    };

    thread = threads[thread_count];
    thread_count++;

    // Creating a thread to handle client communication
    if (pthread_create(&thread, NULL, handleClient, (void*)&params) != 0) {
      printf("SERVER: Failed to create client socket thread.\n");
      free(threads);
      return -1;
    }
  }

  // Cleanup and shutdown
  close(server_socket);
  free(threads);
  printf("SERVER: Shutting down.\n");

  return 0;
}