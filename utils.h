#include <stdio.h>
#include <netinet/in.h>

#define MAX_LINE_LENGTH 100
#define TYPE_STRING_LENGTH 15

struct Pokemon { // Define a structure to hold Pokemon data
  int id;
  char name[75];
  char type1[TYPE_STRING_LENGTH];
  char type2[TYPE_STRING_LENGTH];
  int total;
  int hp;
  int attack;
  int defense;
  int sp_attack;
  int sp_def;
  int speed;
  int generation;
  char legendary[6];
};

int getFile(FILE **file, char **filename, char *permission, char *display_message);
int readPokemonData(FILE *file, struct Pokemon **pokemon_array, int *pokemon_count);
void serializePokemonArray(struct Pokemon *pokemonArray, int count, uint8_t **serializedData, size_t *serializedSize);
int deserializePokemonArray(uint8_t *serializedData, size_t dataSize, struct Pokemon **pokemonArray, int *count);