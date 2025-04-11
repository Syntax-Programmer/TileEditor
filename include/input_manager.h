#pragma once

#include "../include/common.h"

#define INPUT_CHAR_BITMASK 24

typedef enum Enum_Inputs {
  UP = 1 << 0,
  DOWN = 1 << 1,
  LEFT = 1 << 2,
  RIGHT = 1 << 3,
  BACKSPACE = 1 << 4,
  ENTER = 1 << 5,
  QUIT = 1 << 6,
  MSB = 1 << 7,
  SCROLL_UP = 1 << 8,
  SCROLL_DOWN = 1 << 9
} Enum_Inputs;

extern Enum_Inputs GetInput(uint32_t *pRecorded_mouse_click_x,
                            uint32_t *pRecorded_mouse_click_y);