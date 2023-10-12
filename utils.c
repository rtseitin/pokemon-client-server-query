#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

/**************************************************************/
/* Purpose: Get a file pointer and filename from the user, check if the file exists, and open it with the specified permission.

Input:
- permission: Pointer to a character array to specify the file open permission mode (e.g., "r" for read, "w" for write).
- display_message: Pointer to a character array to specify the display message shown if an invalid file is provided by the user

Output:
- file: Pointer to a FILE pointer to store the file handle.
- filename: Pointer to a char pointer to store the filename entered by the user.

Return:
- 0 on success (file exists and opened).
- 1 if the user wants to exit or if an error occurred.
*/
/**************************************************************/
int getFile(FILE **file, char **filename, char *permission, char *display_message) {
  char *file_path = NULL; // Dynamic memory to hold the entered file path
  size_t len = 0;

  while (!(*file)) {
    printf("Enter a filename: ");
    
    char buffer[100]; // Temporary buffer to hold input
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      len = strlen(buffer);
      if (buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        --len;
      }
      
      file_path = realloc(file_path, len + 1); // Allocate exact memory needed
      strcpy(file_path, buffer); // Copy the string from buffer to file_path
    }

    if (file_path == NULL) {
      free(file_path);
      return -1;
    }

    if (strcmp(file_path, "exit") == 0) {
      free(file_path);
      return 1;
    }

    *file = fopen(file_path, permission); // Try to open the file with the specified permission
    *filename = file_path; // Save the file path into the variable pointed by filename

    if (!(*file)) {
      printf("\n%s\n", display_message);
      printf("Or, \"exit\" to quit.\n\n");
    }
  }

  return 0;
}

/**************************************************************/
/* Purpose: Read Pokemon data from a file and populate the pokemon_array.

Input:
- file: Pointer to a FILE structure representing the opened file.

Output:
- pokemon_array: Pointer to a pointer of a struct Pokemon array to store the data.
- pokemon_count: Pointer to an integer to store the number of Pokemon read.

The function populates the pokemon_array and updates pokemon_count.

Return:
- 0 on success (data parsed successfully).
- 1 if an error occurred.

Note: The memory for pokemon_array is allocated and reallocated inside this function.
And will need to be freed outside its confines
*/
/**************************************************************/
int readPokemonData(FILE *file, struct Pokemon **pokemon_array, int *pokemon_count) {
  char line[MAX_LINE_LENGTH];
  fgets(line, sizeof(line), file); // Skip the header line

  while (fgets(line, sizeof(line), file) != NULL) {
    // Reallocate memory for the pokemon_array to accommodate the new Pokemon data
    *pokemon_array = (struct Pokemon*) realloc(*pokemon_array, sizeof(struct Pokemon) * (*pokemon_count + 1));

    if (*pokemon_array == NULL) {
      printf("Failed to allocate memory.\n");
      fclose(file);
      free(*pokemon_array);
      return 1;
    }

    struct Pokemon pokemon;

    // Parse each line of the file and store the data in the pokemon structure
    sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%s",
           &pokemon.id, pokemon.name, pokemon.type1, pokemon.type2, &pokemon.total,
           &pokemon.hp, &pokemon.attack, &pokemon.defense, &pokemon.sp_attack, &pokemon.sp_def,
           &pokemon.speed, &pokemon.generation, pokemon.legendary);

    // Add the newly read Pokemon to the pokemon_array
    (*pokemon_array)[*pokemon_count] = pokemon;
    (*pokemon_count)++;
  }

  // Return 0 on success, indicating that the function ran without errors
  return 0;
}

/**************************************************************/
/* Purpose: Seralize an array of Pokemon structs to a binary data format.

Input:
- pokemon_array: Pointer to an array of struct Pokemon containing the data to be serialized.
- count: Number of Pokemon structs in the array.

Output:
- serialized_data: Pointer to a pointer where the serialized data will be stored.
- serialized_size: Pointer to a size_t variable where the size of the serialized data will be stored.
*/
/**************************************************************/
void serializePokemonArray(struct Pokemon *pokemon_array, int count, uint8_t **serialized_data, size_t *serialized_size) {
  size_t size_per_pokemon = sizeof(struct Pokemon);
  *serialized_size = sizeof(int) + (size_per_pokemon * count);
  *serialized_data = (uint8_t *)malloc(*serialized_size);

  // Serialize the count
  int count_seralized = htonl(count);
  memcpy(*serialized_data, &count_seralized, sizeof(int));

  // Serialize each Pokemon entry
  for (int i = 0; i < count; ++i) {
    memcpy(*serialized_data + sizeof(int) + (i * size_per_pokemon), &pokemon_array[i], size_per_pokemon);
  }
}

/**************************************************************/
/* Purpose: Deseralizes binary data to an array of Pokemon structs.

Input:
- serialized_data: Pointer to the serialized data.
- data_size: Size of the serialized data.

Output:
- pokemon_array: Pointer to a pointer where the deserialized Pokemon array will be stored.
- count: Pointer to an integer where the number of deserialized Pokemon structs will be stored.

Return:
- 0 on success.
- -1 if memory allocation fails.
*/
/**************************************************************/
int deserializePokemonArray(uint8_t *serialized_data, size_t data_size, struct Pokemon **pokemon_array, int *count) {
  // Deserialize the count
  int count_seralized;
  memcpy(&count_seralized, serialized_data, sizeof(int));
  *count = ntohl(count_seralized);

  // Allocate memory for Pokemon array
  *pokemon_array = (struct Pokemon *)malloc(sizeof(struct Pokemon) * (*count));

  if (*pokemon_array == NULL) return -1;

  // Deserialize each Pokemon entry
  size_t size_per_pokemon = sizeof(struct Pokemon);
  for (int i = 0; i < *count; ++i) {
    memcpy(&((*pokemon_array)[i]), serialized_data + sizeof(int) + (i * size_per_pokemon), size_per_pokemon);
  }

  return 0;
}
