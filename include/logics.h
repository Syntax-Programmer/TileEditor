#pragma once

#include "../include/common.h"
#include <SDL2/SDL_rect.h>

extern Enum_StatusCodes PointRectCollision(SDL_Rect *pRect, int32_t point_x,
                                           int32_t point_y);
extern void GetGridPos(uint32_t screen_x, uint32_t screen_y, uint32_t *pGrid_x,
                       uint32_t *pGrid_y);