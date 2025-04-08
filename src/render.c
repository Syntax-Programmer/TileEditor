#include "../include/render.h"

static void RenderGrid(SDL_Renderer *renderer,
                       Struct_TileHashNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset);
static void RenderNumInputWidget(SDL_Renderer *renderer,
                                 Struct_NumInputWidget *pNum_input_widget);
static void
RenderSelectedColorState(SDL_Renderer *renderer,
                         Struct_SelectedColorState *pSelected_color_state);

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

static void RenderNumInputWidget(SDL_Renderer *renderer,
                                 Struct_NumInputWidget *pNum_input_widget) {
  if (pNum_input_widget->widget_texture) {
    SDL_RenderCopy(renderer, pNum_input_widget->widget_texture, NULL,
                   &pNum_input_widget->widget_pos);
  }
}

static void
RenderSelectedColorState(SDL_Renderer *renderer,
                         Struct_SelectedColorState *pSelected_color_state) {
  SDL_SetRenderDrawColor(renderer, pSelected_color_state->r.value,
                         pSelected_color_state->g.value,
                         pSelected_color_state->b.value, 255);
  SDL_RenderFillRect(renderer, &pSelected_color_state->color_rect);
  RenderNumInputWidget(renderer, &pSelected_color_state->r);
  RenderNumInputWidget(renderer, &pSelected_color_state->g);
  RenderNumInputWidget(renderer, &pSelected_color_state->b);
}

void Render(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
            Struct_SelectedColorState *pSelected_color_state,
            int32_t move_x_offset, int32_t move_y_offset) {
  SDL_RenderClear(renderer);
  RenderGrid(renderer, tile_hash_arr, move_x_offset, move_y_offset);
  RenderSelectedColorState(renderer, pSelected_color_state);
  SDL_SetRenderDrawColor(renderer, WHITISH, 255);
  SDL_RenderPresent(renderer);
}