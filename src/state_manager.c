#include "../include/state_manager.h"
#include "../include/logics.h"

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileHashNode **tile_hash_arr,
                             Struct_SelectedColorState *pSelected_color_state,
                             int32_t move_x_offset, int32_t move_y_offset);
static void HandleNumInputWidgetClicks(SDL_Renderer *renderer,
                                       Enum_Inputs input_flags,
                                       Struct_NumInputWidget *pNum_input_widget,
                                       uint32_t *pMax_val);
static void
HandleSelectedColorState(SDL_Renderer *renderer, Enum_Inputs input_flags,
                         Struct_SelectedColorState *pSelected_color_state,
                         uint32_t *pRecorded_mouse_click_x,
                         uint32_t *pRecorded_mouse_click_y);
static void HandleGridMoving(uint32_t input_flags, int32_t *pMove_x_offset,
                             int32_t *pMove_y_offset);

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileHashNode **tile_hash_arr,
                             Struct_SelectedColorState *pSelected_color_state,
                             int32_t move_x_offset, int32_t move_y_offset) {
  if (PopTileHashMapEntry(grid_x + move_x_offset, grid_y + move_y_offset,
                          tile_hash_arr) == SUCCESS) {
    return;
  } else {
    AddTileHashMapEntry(grid_x + move_x_offset, grid_y + move_y_offset,
                        pSelected_color_state->r.value,
                        pSelected_color_state->g.value,
                        pSelected_color_state->b.value, tile_hash_arr);
  }
}

static void HandleNumInputWidgetClicks(SDL_Renderer *renderer,
                                       Enum_Inputs input_flags,
                                       Struct_NumInputWidget *pNum_input_widget,
                                       uint32_t *pMax_val) {
  /*
  Can pass NULL pointer to the widget. This will signify that the no widget is
  being edited.
  */
  char keypress[2];
  if (!pNum_input_widget ||
      ((isdigit(keypress[0] = input_flags >> INPUT_CHAR_BITMASK) &&
        (HAS_FLAG(input_flags, BACKSPACE))) ||
       (!isdigit(keypress[0]) && !HAS_FLAG(input_flags, BACKSPACE)))) {
    /*
    Skipping unneeded computation if (both digit and backspace is pressed) or
    (neither is pressed)
    */
    return;
  } else {
    keypress[1] = '\0';
  }

  uint32_t new_val = pNum_input_widget->value;

  if (HAS_FLAG(input_flags, BACKSPACE)) {
    new_val /= 10;
  } else if (isdigit(keypress[0])) {
    new_val *= 10;
    new_val += atoi(keypress);
  }

  if (pMax_val && new_val > *pMax_val) {
    new_val = pNum_input_widget->value;
  }
  if (pNum_input_widget->widget_texture) {
    SDL_DestroyTexture(pNum_input_widget->widget_texture);
  }
  EditNumInputWidgetValue(pNum_input_widget, renderer, new_val);
}

static void
HandleSelectedColorState(SDL_Renderer *renderer, Enum_Inputs input_flags,
                         Struct_SelectedColorState *pSelected_color_state,
                         uint32_t *pRecorded_mouse_click_x,
                         uint32_t *pRecorded_mouse_click_y) {
  /*
  Choosing which widget to edit.
  Also updating the mouse click pos to the x,y of the numinputwidget to avoid
  the backspace issue.
  */
  if (pSelected_color_state->selected &&
      PointRectCollision(&pSelected_color_state->selected->widget_pos,
                         *pRecorded_mouse_click_x,
                         *pRecorded_mouse_click_y) == SUCCESS) {
    // No new clicks
  } else if (PointRectCollision(&pSelected_color_state->r.widget_pos,
                                *pRecorded_mouse_click_x,
                                *pRecorded_mouse_click_y) == SUCCESS) {
    pSelected_color_state->selected = &pSelected_color_state->r;
    *pRecorded_mouse_click_x = pSelected_color_state->selected->widget_pos.x;
    *pRecorded_mouse_click_y = pSelected_color_state->selected->widget_pos.y;
  } else if (PointRectCollision(&pSelected_color_state->g.widget_pos,
                                *pRecorded_mouse_click_x,
                                *pRecorded_mouse_click_y) == SUCCESS) {
    pSelected_color_state->selected = &pSelected_color_state->g;
    *pRecorded_mouse_click_x = pSelected_color_state->selected->widget_pos.x;
    *pRecorded_mouse_click_y = pSelected_color_state->selected->widget_pos.y;
  } else if (PointRectCollision(&pSelected_color_state->b.widget_pos,
                                *pRecorded_mouse_click_x,
                                *pRecorded_mouse_click_y) == SUCCESS) {
    pSelected_color_state->selected = &pSelected_color_state->b;
    *pRecorded_mouse_click_x = pSelected_color_state->selected->widget_pos.x;
    *pRecorded_mouse_click_y = pSelected_color_state->selected->widget_pos.y;
  } else if (HAS_FLAG(input_flags, ENTER)) {
    pSelected_color_state->selected = NULL;
  } else {
    pSelected_color_state->selected = NULL;
  }

  uint32_t max_color_val = 255;
  HandleNumInputWidgetClicks(renderer, input_flags,
                             pSelected_color_state->selected, &max_color_val);
}

static void HandleGridMoving(uint32_t input_flags, int32_t *pMove_x_offset,
                             int32_t *pMove_y_offset) {
  if (HAS_FLAG(input_flags, UP)) {
    *pMove_y_offset -= TILE_SIZE;
  }
  if (HAS_FLAG(input_flags, DOWN)) {
    *pMove_y_offset += TILE_SIZE;
  }
  if (HAS_FLAG(input_flags, LEFT)) {
    *pMove_x_offset -= TILE_SIZE;
  }
  if (HAS_FLAG(input_flags, RIGHT)) {
    *pMove_x_offset += TILE_SIZE;
  }
}

void HandleState(SDL_Renderer *renderer, Struct_TileHashNode **tile_hash_arr,
                 Struct_SelectedColorState *pSelected_color_state,
                 Enum_Inputs input_flags, int32_t *pMove_x_offset,
                 int32_t *pMove_y_offset, uint32_t *pRecorded_mouse_click_x,
                 uint32_t *pRecorded_mouse_click_y, uint32_t *pCurrent_time) {
  if (HAS_FLAG(input_flags, MSB) && *pRecorded_mouse_click_x <= GRID_WIDTH) {
    GetGridPos(*pRecorded_mouse_click_x, *pRecorded_mouse_click_y,
               pRecorded_mouse_click_x, pRecorded_mouse_click_y);
    HandleTileClicks(*pRecorded_mouse_click_x, *pRecorded_mouse_click_y,
                     tile_hash_arr, pSelected_color_state, *pMove_x_offset,
                     *pMove_y_offset);
  } else if ((HAS_FLAG(input_flags, MSB) &&
              *pRecorded_mouse_click_x > GRID_WIDTH) ||
             pSelected_color_state->selected) {
    HandleSelectedColorState(renderer, input_flags, pSelected_color_state,
                             pRecorded_mouse_click_x, pRecorded_mouse_click_y);
  }
  // This means we are not currently editing rgb and input delay is covered.
  if (!pSelected_color_state->selected &&
      SDL_GetTicks() - *pCurrent_time >= INPUT_DELAY) {
    *pCurrent_time = SDL_GetTicks();
    HandleGridMoving(input_flags, pMove_x_offset, pMove_y_offset);
  }
}