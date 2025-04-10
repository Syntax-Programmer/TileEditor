#include "../include/state_manager.h"
#include "../include/logics.h"

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileHashNode **tile_hash_arr,
                             Struct_InputWidgetState *pInput_widget_state,
                             int32_t move_x_offset, int32_t move_y_offset);
static void HandleInputWidgetClicks(SDL_Renderer *renderer,
                                    Enum_Inputs input_flags,
                                    Struct_InputWidget *pInput_widget);
static void HandleInputWidgetState(SDL_Renderer *renderer,
                                   Enum_Inputs input_flags,
                                   Struct_InputWidgetState *pInput_widget_state,
                                   uint32_t *pRecorded_mouse_click_x,
                                   uint32_t *pRecorded_mouse_click_y);
static void HandleGridMoving(uint32_t input_flags, int32_t *pMove_x_offset,
                             int32_t *pMove_y_offset);

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileHashNode **tile_hash_arr,
                             Struct_InputWidgetState *pInput_widget_state,
                             int32_t move_x_offset, int32_t move_y_offset) {
  if (PopTileHashMapEntry(grid_x + move_x_offset, grid_y + move_y_offset,
                          tile_hash_arr) == SUCCESS) {
    return;
  } else {
    AddTileHashMapEntry(
        grid_x + move_x_offset, grid_y + move_y_offset,
        pInput_widget_state->widgets[R_WIDGET_INDEX].Value.int_val,
        pInput_widget_state->widgets[G_WIDGET_INDEX].Value.int_val,
        pInput_widget_state->widgets[B_WIDGET_INDEX].Value.int_val,
        tile_hash_arr);
  }
}

static void HandleInputWidgetClicks(SDL_Renderer *renderer,
                                    Enum_Inputs input_flags,
                                    Struct_InputWidget *pInput_widget) {
  /*
  Can pass NULL pointer to the widget. This will signify that the no widget is
  being edited.
  */
  char keypress;
  keypress = input_flags >> INPUT_CHAR_BITMASK;
  if (!pInput_widget) {
    // Due to the eay the input_manager is set up, below conditions never
    // actually arise
    //  || (keypress && HAS_FLAG(input_flags, BACKSPACE)) ||
    //  (!keypress && !HAS_FLAG(input_flags, BACKSPACE))) {

    /*
    Skipping unneeded computation if (both digit and backspace is pressed) or
    (neither is pressed)
    */
    return;
  }

  int32_t int_val = 0;
  char str_val[WIDGET_CHARACTER_LIMIT];
  int32_t str_val_len;

  if (pInput_widget->ValueType == INT) {
    int_val = pInput_widget->Value.int_val;
  } else if (pInput_widget->ValueType == STR) {
    snprintf(str_val, sizeof(str_val), "%s", pInput_widget->Value.str_val);
    str_val_len = strlen(str_val);
  }

  if (HAS_FLAG(input_flags, BACKSPACE)) {
    if (pInput_widget->ValueType == INT) {
      int_val /= 10;
    } else if (pInput_widget->ValueType == STR && str_val_len > 0) {
      str_val[str_val_len - 1] = '\0';
    }
  } else if (keypress) {
    if (pInput_widget->ValueType == INT) {
      if (isdigit(keypress)) {
        int_val *= 10;
        int_val += (keypress - '0');
      } else if (keypress == '-') {
        int_val *= -1;
      }
    } else if (pInput_widget->ValueType == STR) {
      str_val[str_val_len] = keypress;
      str_val[str_val_len + 1] = '\0';
    }
  }

  EditInputWidget(pInput_widget, renderer,
                  (pInput_widget->ValueType == STR) ? str_val : NULL,
                  (pInput_widget->ValueType == INT) ? &int_val : NULL);
}

static void HandleInputWidgetState(SDL_Renderer *renderer,
                                   Enum_Inputs input_flags,
                                   Struct_InputWidgetState *pInput_widget_state,
                                   uint32_t *pRecorded_mouse_click_x,
                                   uint32_t *pRecorded_mouse_click_y) {
  /*
  Choosing which widget to edit.
  Also updating the mouse click pos to the x,y of the numinputwidget to avoid
  the backspace issue.
  */
  if (pInput_widget_state->selected &&
      PointRectCollision(&pInput_widget_state->selected->pos,
                         *pRecorded_mouse_click_x,
                         *pRecorded_mouse_click_y) == SUCCESS) {
    // No new clicks
  } else if (HAS_FLAG(input_flags, ENTER)) {
    pInput_widget_state->selected = NULL;
  } else {
    pInput_widget_state->selected = NULL;
    for (int i = 0; i < MAX_WIDGETS; i++) {
      if (PointRectCollision(&pInput_widget_state->widgets[i].pos,
                             *pRecorded_mouse_click_x,
                             *pRecorded_mouse_click_y) == SUCCESS) {
        pInput_widget_state->selected = &pInput_widget_state->widgets[i];
        *pRecorded_mouse_click_x = pInput_widget_state->selected->pos.x;
        *pRecorded_mouse_click_y = pInput_widget_state->selected->pos.y;
        break;
      }
    }
  }

  HandleInputWidgetClicks(renderer, input_flags, pInput_widget_state->selected);
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
                 Struct_InputWidgetState *pInput_widget_state,
                 Enum_Inputs input_flags, int32_t *pMove_x_offset,
                 int32_t *pMove_y_offset, uint32_t *pRecorded_mouse_click_x,
                 uint32_t *pRecorded_mouse_click_y, uint32_t *pCurrent_time) {
  if (HAS_FLAG(input_flags, MSB) && *pRecorded_mouse_click_x <= GRID_WIDTH) {
    GetGridPos(*pRecorded_mouse_click_x, *pRecorded_mouse_click_y,
               pRecorded_mouse_click_x, pRecorded_mouse_click_y);
    HandleTileClicks(*pRecorded_mouse_click_x, *pRecorded_mouse_click_y,
                     tile_hash_arr, pInput_widget_state, *pMove_x_offset,
                     *pMove_y_offset);
  } else if ((HAS_FLAG(input_flags, MSB) &&
              *pRecorded_mouse_click_x > GRID_WIDTH) ||
             pInput_widget_state->selected) {
    HandleInputWidgetState(renderer, input_flags, pInput_widget_state,
                           pRecorded_mouse_click_x, pRecorded_mouse_click_y);
  }
  // This means we are not currently editing rgb and input delay is covered.
  if (!pInput_widget_state->selected &&
      SDL_GetTicks() - *pCurrent_time >= INPUT_DELAY) {
    *pCurrent_time = SDL_GetTicks();
    HandleGridMoving(input_flags, pMove_x_offset, pMove_y_offset);
  }
}