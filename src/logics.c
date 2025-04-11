#include "../include/logics.h"

Enum_StatusCodes PointRectCollision(SDL_Rect *pRect, int32_t point_x,
                                    int32_t point_y) {
  if (point_x >= pRect->x && point_x <= pRect->x + pRect->w &&
      point_y >= pRect->y && point_y <= pRect->y + pRect->h) {
    return SUCCESS;
  }

  return FAILURE;
}

void GetGridIndex(uint32_t screen_x, uint32_t screen_y, uint32_t *pGrid_x_index,
                  uint32_t *pGrid_y_index) {
  *pGrid_x_index = (screen_x / grid_size);
  *pGrid_y_index = (screen_y / grid_size);
}