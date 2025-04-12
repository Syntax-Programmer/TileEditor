#include "../include/render.h"

static SDL_Rect OUTSIDE_GRID = {
    .x = GRID_WIDTH, .y = 0, .w = APP_WIDTH - GRID_WIDTH, .h = APP_HEIGHT};

static void RenderGrid(SDL_Renderer *renderer,
                       Struct_TileHashNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset);

static void
RenderInputWidgets(SDL_Renderer *renderer,
                   const Struct_InputWidgetState *pInput_widget_state);

static void RenderGrid(SDL_Renderer *renderer,
                       Struct_TileHashNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset) {
  SDL_Rect rect = {.w = grid_size, .h = grid_size};
  Struct_TileHashNode *temp = NULL;

  /*
  Using this way, when zooming in, the grid cuts off without covering the
  screen.

  Fixing this using the way of +1 rows and +1 cols than calculated is not viable
  as doing it will take unallocated space on screen and also may coverup some
  widgets.
  */
  uint32_t rows = (GRID_HEIGHT / grid_size) + 1,
           cols = (GRID_WIDTH / grid_size) + 1;

  for (uint32_t i = 0; i < rows; i++) {
    for (uint32_t j = 0; j < cols; j++) {
      rect.y = i * grid_size;
      rect.x = j * grid_size;
      if (AccessTileHashMap(j + move_x_offset, i + move_y_offset, tile_hash_arr,
                            &temp) == SUCCESS) {
        SDL_SetRenderDrawColor(renderer, temp->r, temp->g, temp->b, 255);
        SDL_RenderFillRect(renderer, &rect);
      } else {
        SDL_SetRenderDrawColor(renderer, BLACKISH, 255);
        SDL_RenderDrawRect(renderer, &rect);
      }
    }
  }
}

static void
RenderInputWidgets(SDL_Renderer *renderer,
                   const Struct_InputWidgetState *pInput_widget_state) {
  for (int32_t i = 0; i < MAX_WIDGETS; i++) {
    if (pInput_widget_state->widgets[i].texture) {
      SDL_RenderCopy(renderer, pInput_widget_state->widgets[i].texture, NULL,
                     &pInput_widget_state->widgets[i].pos);
    }
  }
}

void Render(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
            const Struct_InputWidgetState *pInput_widget_state,
            int32_t move_x_offset, int32_t move_y_offset) {
  SDL_RenderClear(renderer);

  RenderGrid(renderer, tile_hash_arr, move_x_offset, move_y_offset);

  SDL_SetRenderDrawColor(renderer, WHITISH, 255);
  SDL_RenderFillRect(renderer, &OUTSIDE_GRID);

  RenderInputWidgets(renderer, pInput_widget_state);

  SDL_SetRenderDrawColor(
      renderer, pInput_widget_state->widgets[R_WIDGET_INDEX].Value.int_val,
      pInput_widget_state->widgets[G_WIDGET_INDEX].Value.int_val,
      pInput_widget_state->widgets[B_WIDGET_INDEX].Value.int_val, 255);
  SDL_RenderFillRect(renderer, &COLOR_RECT);

  SDL_SetRenderDrawColor(renderer, WHITISH, 255);
  SDL_RenderPresent(renderer);
}