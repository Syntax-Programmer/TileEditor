#include "../include/tile_map_manager.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define KNUTHS_X_MULTIPLIER 2654435761U
#define KNUTHS_Y_MULTIPLIER 2246822519U
#define HASH_BUCKET_SIZE 5000

#define MAX_LINE_SIZE 40
#define PERLINE_ATTR_COUNT 5
#define LINES_PER_RECT 6

static uint32_t KnuthMultiplicativeHash(int32_t x, int32_t y);
static Enum_StatusCodes ParseVDataLine(char *data_line,
                                       Struct_TileHashNode **tile_hash_arr);

static uint32_t KnuthMultiplicativeHash(int32_t x, int32_t y) {
  uint32_t ux = (uint32_t)x * KNUTHS_X_MULTIPLIER;
  uint32_t uy = (uint32_t)y * KNUTHS_Y_MULTIPLIER;
  uint32_t raw_hash =
      (ux ^ (uy >> 16) ^ (uy << 13) ^ (x >> 5) ^ (y << 7)); // Extra mixing

  return raw_hash % HASH_BUCKET_SIZE;
}

Enum_StatusCodes InitTileHashMap(Struct_TileHashNode ***pTile_hash_arr) {
  Enum_StatusCodes status = SUCCESS;

  *pTile_hash_arr = calloc(HASH_BUCKET_SIZE, sizeof(Struct_TileHashNode *));
  if (!(*pTile_hash_arr)) {
    status = MEM_ALLOC_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, NULL, "Error produced by InitTileHashMap()",
           OUTPUT_LOG_STREAM);
    return status;
  }

  return status;
}

void FreeTileHashMap(Struct_TileHashNode ***pTile_hash_arr) {
  Struct_TileHashNode *temp;

  for (int i = 0; i < HASH_BUCKET_SIZE; i++) {
    while ((*pTile_hash_arr)[i]) {
      temp = (*pTile_hash_arr)[i]->next;
      free((*pTile_hash_arr)[i]);
      (*pTile_hash_arr)[i] = temp;
    }
  }
  free(*pTile_hash_arr);
  (*pTile_hash_arr) = NULL;
}

Enum_StatusCodes AddTileHashMapEntry(int32_t x, int32_t y, uint8_t r, uint8_t g,
                                     uint8_t b,
                                     Struct_TileHashNode **tile_hash_arr) {
  Enum_StatusCodes status = SUCCESS;
  Struct_TileHashNode *node = malloc(sizeof(Struct_TileHashNode));

  if (!node) {
    status = MEM_ALLOC_FAILURE | LOW_SEVERITY_ERROR;
    Logger(&status, NULL, "Error produced by AddTileHashMapEntry()",
           OUTPUT_LOG_STREAM);
    return status;
  }
  node->x = x;
  node->y = y;
  node->r = r;
  node->g = g;
  node->b = b;

  uint32_t index = KnuthMultiplicativeHash(x, y);

  node->next = tile_hash_arr[index];
  tile_hash_arr[index] = node;

  return status;
}

Enum_StatusCodes AccessTileHashMap(int32_t x, int32_t y,
                                   Struct_TileHashNode **tile_hash_arr,
                                   Struct_TileHashNode **pDest) {
  uint32_t index = KnuthMultiplicativeHash(x, y);

  *pDest = tile_hash_arr[index];
  while (*pDest) {
    if ((*pDest)->x == x && (*pDest)->y == y) {
      return SUCCESS;
    }
    *pDest = (*pDest)->next;
  }
  *pDest = NULL;

  return FAILURE;
}

Enum_StatusCodes PopTileHashMapEntry(int32_t x, int32_t y,
                                     Struct_TileHashNode **tile_hash_arr) {
  uint32_t index = KnuthMultiplicativeHash(x, y);
  Struct_TileHashNode *curr = tile_hash_arr[index];
  Struct_TileHashNode *prev = NULL;

  while (curr) {
    if (curr->x == x && curr->y == y) {
      if (!prev) {
        tile_hash_arr[index] = curr->next;
      } else {
        prev->next = curr->next;
      }
      free(curr);
      return SUCCESS;
    }
    prev = curr;
    curr = curr->next;
  }

  return FAILURE;
}

Enum_StatusCodes DumpDataToFile(Struct_TileHashNode **tile_hash_arr,
                                const char *file_path) {
  Enum_StatusCodes status = SUCCESS;

  FILE *file = fopen(file_path, "w");
  if (!file) {
    status = INVALID_FILE_PATH | HIGH_SEVERITY_ERROR;
    Logger(&status, NULL, "Error produced by DumpDataToFile()",
           OUTPUT_LOG_STREAM);
    return status;
  }

  int32_t vert_c = 0;
  for (int i = 0; i < HASH_BUCKET_SIZE; i++) {
    Struct_TileHashNode *curr = tile_hash_arr[i];
    while (curr) {
      fprintf(file,
              "\nv %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "i %d %d %d\n"
              "i %d %d %d\n",
              curr->x, curr->y, curr->r, curr->g, curr->b, curr->x + TILE_SIZE,
              curr->y, curr->r, curr->g, curr->b, curr->x, curr->y + TILE_SIZE,
              curr->r, curr->g, curr->b, curr->x + TILE_SIZE,
              curr->y + TILE_SIZE, curr->r, curr->g, curr->b, vert_c + 0,
              vert_c + 1, vert_c + 3, vert_c + 0, vert_c + 2, vert_c + 3);
      curr = curr->next;
      vert_c += 4;
    }
  }
  fclose(file);

  return status;
}

Enum_StatusCodes ParseVDataLine(char *data_line,
                                Struct_TileHashNode **tile_hash_arr) {
  Enum_StatusCodes status = SUCCESS;
  char *token = strtok(&data_line[2], " \n");
  uint32_t i = 0;
  int32_t x, y;
  uint8_t r, g, b;

  while (token && i < PERLINE_ATTR_COUNT) {
    char *end_ptr;
    int32_t data_token = strtol(token, &end_ptr, 10);
    if (end_ptr[0] != '\0' || end_ptr == token) {
      status = UNEXPECTED_COMPUTED_RESULTS | HIGH_SEVERITY_ERROR;
      Logger(&status, NULL, data_line, OUTPUT_LOG_STREAM);
      return status;
    }
    switch (i) {
    case 0:
      x = data_token;
      break;
    case 1:
      y = data_token;
      break;
    case 2:
      r = data_token;
      break;
    case 3:
      g = data_token;
      break;
    case 4:
      b = data_token;
      break;
    default:
      break;
    }
    token = strtok(NULL, " \n");
    i++;
  }
  if (i < PERLINE_ATTR_COUNT) {
    status = UNEXPECTED_COMPUTED_RESULTS | HIGH_SEVERITY_ERROR;
    Logger(&status, NULL, data_line, OUTPUT_LOG_STREAM);
    return status;
  }

  return AddTileHashMapEntry(x, y, r, g, b, tile_hash_arr);
}

Enum_StatusCodes ParseFileToData(Struct_TileHashNode **tile_hash_arr,
                                 const char *file_path) {
  Enum_StatusCodes status = SUCCESS;
  FILE *file = fopen(file_path, "r");

  if (!file) {
    status = INVALID_FILE_PATH | HIGH_SEVERITY_ERROR;
    Logger(&status, NULL, "Error produced by ParseFileToData()",
           OUTPUT_LOG_STREAM);
    return status;
  }

  char buffer[MAX_LINE_SIZE];

  while (fgets(buffer, sizeof(buffer), file)) {
    if (buffer[strlen(buffer) - 1] != '\n' && !feof(file)) {
      // Digesting the entire line to ignore.
      char temp;
      while ((temp = fgetc(file)) != '\n' && temp != EOF)
        ;
      continue;
    } else if (buffer[0] == 'v' && buffer[1] == ' ') {
      if ((status = ParseVDataLine(buffer, tile_hash_arr)) != SUCCESS) {
        fclose(file);
        return status;
      }
      // Digesting vertices and indices that makeup the rect and just directly
      // building it here. Assuming data correctness.
      for (int i = 0; i < LINES_PER_RECT; i++) {
        if (!fgets(buffer, MAX_LINE_SIZE, file) && !feof(file)) {
          status = FILE_IO_ERROR | HIGH_SEVERITY_ERROR;
          Logger(&status, NULL, "Error produced by ParseFileToData()",
                 OUTPUT_LOG_STREAM);
          fclose(file);
          return status;
        }
      }
    }
  }
  fclose(file);

  return SUCCESS;
}