#pragma once

#include "../include/gfx.h"
#include "../include/input_manager.h"
#include "../include/tile_map_manager.h"

extern void
HandleState(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
            Struct_SelectedColorState *pSelected_color_state,
            Enum_Inputs input_flags, int32_t *pMove_x_offset,
            int32_t *pMove_y_offset, uint32_t *pRecorded_mouse_click_x,
            uint32_t *pRecorded_mouse_click_y, uint32_t *pCurrent_time);