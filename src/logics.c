#include "../include/logics.h"

Enum_StatusCodes PointRectCollision(SDL_Rect *pRect, int32_t point_x,
                                    int32_t point_y) {
  if (point_x >= pRect->x && point_x <= pRect->x + pRect->w &&
      point_y >= pRect->y && point_y <= pRect->y + pRect->h) {
    return SUCCESS;
  }

  return FAILURE;
}

void GetGridPos(uint32_t screen_x, uint32_t screen_y, uint32_t *pGrid_x,
                uint32_t *pGrid_y) {
  *pGrid_x = (screen_x / TILE_SIZE) * TILE_SIZE;
  *pGrid_y = (screen_y / TILE_SIZE) * TILE_SIZE;
}