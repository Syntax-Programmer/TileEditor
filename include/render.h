#pragma once

#include "../include/gfx.h"
#include "../include/tile_map_manager.h"

extern void Render(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
                   Struct_SelectedColorState *pSelected_color_state,
                   int32_t move_x_offset, int32_t move_y_offset);