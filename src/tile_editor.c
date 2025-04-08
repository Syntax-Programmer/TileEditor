#include "../include/render.h"
#include "../include/state_manager.h"

static Enum_StatusCodes
InitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer, TTF_Font **pFont,
        Struct_TileHashNode ***pTile_hash_arr,
        Struct_SelectedColorState *pSelected_color_state);
static void AppLoop(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
                    Struct_SelectedColorState *pSelected_color_state);
static void ExitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                    TTF_Font **pFont, Struct_TileHashNode ***pTile_hash_arr,
                    Struct_SelectedColorState *pSelected_color_state);

static Enum_StatusCodes
InitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer, TTF_Font **pFont,
        Struct_TileHashNode ***pTile_hash_arr,
        Struct_SelectedColorState *pSelected_color_state) {
  if (InitSDL(pWindow, pRenderer) != SUCCESS || InitTTF(pFont) != SUCCESS ||
      InitTileHashMap(pTile_hash_arr) ||
      InitSelectedColorState(pSelected_color_state, *pFont, *pRenderer)) {
    return FAILURE;
  }

  if (ParseFileToData(*pTile_hash_arr, FILE_TO_WORK_ON)) {
    return FAILURE;
  }

  return SUCCESS;
}

static void AppLoop(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
                    Struct_SelectedColorState *pSelected_color_state) {
  uint32_t recorded_mouse_click_x = 0, recorded_mouse_click_y = 0;
  int32_t move_x_offset = 0, move_y_offset = 0;

  uint32_t current_time = SDL_GetTicks();
  Enum_Inputs input_flags = 0;

  while (1) {
    input_flags = GetInput(&recorded_mouse_click_x, &recorded_mouse_click_y);
    if (HAS_FLAG(input_flags, QUIT)) {
      return;
    }
    HandleState(renderer, tile_hash_arr, pSelected_color_state, input_flags,
                &move_x_offset, &move_y_offset, &recorded_mouse_click_x,
                &recorded_mouse_click_y, &current_time);
    if (SDL_GetTicks() - current_time >= FRAME_DELAY) {
      Render(renderer, tile_hash_arr, pSelected_color_state, move_x_offset,
             move_y_offset);
    }
  }
}

static void ExitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                    TTF_Font **pFont, Struct_TileHashNode ***pTile_hash_arr,
                    Struct_SelectedColorState *pSelected_color_state) {
  // If dumping fails, its way before the file was even opend, so no data loss.
  DumpDataToFile(*pTile_hash_arr, FILE_TO_WORK_ON);
  FreeTileHashMap(pTile_hash_arr);
  ExitSelectedColorState(pSelected_color_state);
  ExitTTF(pFont);
  ExitSDL(pWindow, pRenderer);
}

void App(void) {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  TTF_Font *font = NULL;
  Struct_TileHashNode **tile_hash_arr = NULL;
  Struct_SelectedColorState selected_color_state;

  if (InitApp(&window, &renderer, &font, &tile_hash_arr,
              &selected_color_state) == SUCCESS) {
    AppLoop(renderer, tile_hash_arr, &selected_color_state);
  }
  ExitApp(&window, &renderer, &font, &tile_hash_arr, &selected_color_state);
}