#pragma once

#include "../include/common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WIDGET_CHARACTER_LIMIT 128
#define MAX_NUM_WIDGET 4

typedef struct Struct_NumInputWidget {
  uint8_t widget_r, widget_g, widget_b;
  char widget_label_raw[WIDGET_CHARACTER_LIMIT];
  char widget_label[WIDGET_CHARACTER_LIMIT];
  uint32_t value;
  SDL_Rect widget_pos;
  SDL_Texture *widget_texture;
  TTF_Font *widget_font;
} Struct_NumInputWidget;

typedef struct Struct_SelectedColorState {
  Struct_NumInputWidget r, g, b;
  // The rect that will show the selected color.
  SDL_Rect color_rect;
  // This is the selected field frm the r, g, b that is being edited.
  Struct_NumInputWidget *selected;
} Struct_SelectedColorState;

extern Enum_StatusCodes InitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
extern void ExitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
extern Enum_StatusCodes InitTTF(TTF_Font **pFont);
extern void ExitTTF(TTF_Font **pFont);

extern Enum_StatusCodes
CreateNumInputWidget(Struct_NumInputWidget *pNum_input_widget, TTF_Font *font,
                     SDL_Renderer *renderer, uint8_t r, uint8_t g, uint8_t b,
                     char *widget_label, uint32_t widget_value, int32_t pos_x,
                     int32_t pos_y);
extern Enum_StatusCodes
EditNumInputWidgetValue(Struct_NumInputWidget *pNum_input_widget,
                        SDL_Renderer *renderer, uint32_t new_val);
extern void FreeNumInputWidget(Struct_NumInputWidget *pNum_input_widget);

extern Enum_StatusCodes
InitSelectedColorState(Struct_SelectedColorState *pSelected_color_state,
                       TTF_Font *font, SDL_Renderer *renderer);
extern void
ExitSelectedColorState(Struct_SelectedColorState *pSelected_color_state);