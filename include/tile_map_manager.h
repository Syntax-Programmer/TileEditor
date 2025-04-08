#pragma once

#include "../include/common.h"
#include <stdint.h>

typedef struct Struct_TileHashNode {
  int32_t x, y;
  uint8_t r, g, b;
  struct Struct_TileHashNode *next; // Hashmap with chaining.
} Struct_TileHashNode;

extern Enum_StatusCodes InitTileHashMap(Struct_TileHashNode ***pTile_hash_arr);
extern void FreeTileHashMap(Struct_TileHashNode ***pTile_hash_arr);
extern Enum_StatusCodes
AddTileHashMapEntry(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b,
                    Struct_TileHashNode **tile_hash_arr);
extern Enum_StatusCodes AccessTileHashMap(int32_t x, int32_t y,
                                          Struct_TileHashNode **tile_hash_arr,
                                          Struct_TileHashNode **pDest);
extern Enum_StatusCodes
PopTileHashMapEntry(int32_t x, int32_t y, Struct_TileHashNode **tile_hash_arr);

extern Enum_StatusCodes DumpDataToFile(Struct_TileHashNode **tile_hash_arr,
                                       const char *file_path);
extern Enum_StatusCodes ParseFileToData(Struct_TileHashNode **tile_hash_arr,
                                        const char *file_path);
