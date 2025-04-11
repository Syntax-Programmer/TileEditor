#pragma once

#include "../include/common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WIDGET_CHARACTER_LIMIT 128
#define MAX_WIDGETS 3

#define R_WIDGET_INDEX 0
#define G_WIDGET_INDEX 1
#define B_WIDGET_INDEX 2
#define RGB_VAL_LIMIT 0x00FF

typedef struct Struct_InputWidget {
  char title[WIDGET_CHARACTER_LIMIT];
  enum { INT, STR } ValueType;
  union {
    char str_val[WIDGET_CHARACTER_LIMIT];
    int32_t int_val;
  } Value;
  SDL_Color display_color;
  SDL_Rect pos;
  SDL_Texture *texture;
  TTF_Font *font;
} Struct_InputWidget;

static const SDL_Rect COLOR_RECT = {.x = 875, .y = 25, .w = 50, .h = 50};

/*
Current approach is:
Hard coding the number of widgets into a macro.
Then assigning it as an array(arena) of widgets where
each index is a particular widget.

Another approach:
Make a system that grows dynamically during runtime,
determining number of widgets used at runtime.
This is a little inefficient but still a viable option.

To implement the second approach, instead of a macro, make a global var
defining the size of the array that could grow or shrink.
OR
Make an dynamic array of widgets.
*/
typedef struct Struct_InputWidgetState {
  Struct_InputWidget widgets[MAX_WIDGETS];
  Struct_InputWidget *selected;
} Struct_InputWidgetState;

extern Enum_StatusCodes InitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
extern void ExitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
extern Enum_StatusCodes InitTTF(TTF_Font **pFont);
extern void ExitTTF(TTF_Font **pFont);

extern Enum_StatusCodes EditInputWidget(Struct_InputWidget *pInput_widget,
                                        SDL_Renderer *renderer,
                                        const char *new_str_val,
                                        int32_t *pNew_int_val);
extern Enum_StatusCodes
InitInputWidgetState(Struct_InputWidgetState *pInput_widget_state,
                     SDL_Renderer *renderer, TTF_Font *font);
extern void ExitInputWidgetState(Struct_InputWidgetState *pInput_widget_state);