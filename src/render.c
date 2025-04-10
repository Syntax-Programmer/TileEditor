#include "../include/render.h"

static void RenderGrid(SDL_Renderer *renderer,
                       Struct_TileHashNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset);

static void
RenderInputWidgets(SDL_Renderer *renderer,
                   const Struct_InputWidgetState *pInput_widget_state);

static void RenderGrid(SDL_Renderer *renderer,
                       Struct_TileHashNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset) {
  SDL_Rect rect = {.w = TILE_SIZE, .h = TILE_SIZE};
  Struct_TileHashNode *temp = NULL;

  for (int i = 0; i < GRID_HEIGHT; i += TILE_SIZE) {
    for (int j = 0; j < GRID_WIDTH; j += TILE_SIZE) {
      rect.y = i;
      rect.x = j;
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
  for (int i = 0; i < MAX_WIDGETS; i++) {
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
  RenderInputWidgets(renderer, pInput_widget_state);

  SDL_SetRenderDrawColor(
      renderer, pInput_widget_state->widgets[R_WIDGET_INDEX].Value.int_val,
      pInput_widget_state->widgets[G_WIDGET_INDEX].Value.int_val,
      pInput_widget_state->widgets[B_WIDGET_INDEX].Value.int_val, 255);
  SDL_RenderFillRect(renderer, &COLOR_RECT);

  SDL_SetRenderDrawColor(renderer, WHITISH, 255);
  SDL_RenderPresent(renderer);
}