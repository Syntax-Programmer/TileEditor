#include "../include/gfx.h"

Enum_StatusCodes InitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer) {
  Enum_StatusCodes status = SUCCESS;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER)) {
    status = DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, SDL_GetError, "Error originated from InitSDL()",
           OUTPUT_LOG_STREAM);
    return status;
  }

  *pWindow = SDL_CreateWindow(APP_TITLE, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT,
                              SDL_WINDOW_SHOWN);
  if (!(*pWindow)) {
    status = DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, SDL_GetError, "Error originated from InitSDL()",
           OUTPUT_LOG_STREAM);
    SDL_Quit();
    return status;
  }

  *pRenderer = SDL_CreateRenderer(*pWindow, -1, SDL_RENDERER_ACCELERATED);
  if (!(*pRenderer)) {
    status = DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, SDL_GetError, "Error originated from InitSDL()",
           OUTPUT_LOG_STREAM);
    SDL_DestroyWindow(*pWindow);
    SDL_Quit();
    return status;
  }

  return status;
}

void ExitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer) {
  if (pRenderer && *pRenderer) {
    SDL_DestroyRenderer(*pRenderer);
    *pRenderer = NULL;
  }
  if (pWindow && *pWindow) {
    SDL_DestroyWindow(*pWindow);
    *pWindow = NULL;
  }
  SDL_Quit();
}

Enum_StatusCodes InitTTF(TTF_Font **pFont) {
  Enum_StatusCodes status = SUCCESS;

  if (TTF_Init()) {
    status = DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, TTF_GetError, "Error originated from InitTTF()",
           OUTPUT_LOG_STREAM);
    return status;
  }

  *pFont = TTF_OpenFont(APP_FONT, 20);
  if (!(*pFont)) {
    status = DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
    Logger(&status, TTF_GetError, "Error originated from InitTTF()",
           OUTPUT_LOG_STREAM);
    TTF_Quit();
    return status;
  }

  return status;
}

void ExitTTF(TTF_Font **pFont) {
  if (pFont && *pFont) {
    TTF_CloseFont(*pFont);
  }
  TTF_Quit();
}

Enum_StatusCodes CreateNumInputWidget(Struct_NumInputWidget *pNum_input_widget,
                                      TTF_Font *font, SDL_Renderer *renderer,
                                      uint8_t r, uint8_t g, uint8_t b,
                                      char *widget_label, uint32_t widget_value,
                                      int32_t pos_x, int32_t pos_y) {
  Enum_StatusCodes status = SUCCESS;

  pNum_input_widget->value = widget_value;
  pNum_input_widget->widget_r = r;
  pNum_input_widget->widget_g = g;
  pNum_input_widget->widget_b = b;
  pNum_input_widget->widget_font = font;
  snprintf(pNum_input_widget->widget_label_raw,
           sizeof(pNum_input_widget->widget_label_raw), "%s", widget_label);
  snprintf(pNum_input_widget->widget_label,
           sizeof(pNum_input_widget->widget_label), "%s%d", widget_label,
           widget_value);
  pNum_input_widget->widget_pos = (SDL_Rect){.x = pos_x, .y = pos_y};

  SDL_Colour color = {.r = r, .g = g, .b = b, .a = 255};
  SDL_Surface *text_surface =
      TTF_RenderText_Blended(font, pNum_input_widget->widget_label, color);
  if (!text_surface) {
    status = DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR;
    Logger(&status, TTF_GetError, "Error produced by CreateNumInputWidget()",
           stderr);
    return status;
  }

  pNum_input_widget->widget_texture =
      SDL_CreateTextureFromSurface(renderer, text_surface);
  SDL_FreeSurface(text_surface);
  if (!(pNum_input_widget->widget_texture)) {
    status = DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR;
    Logger(&status, SDL_GetError, "Error produced by CreateNumInputWidget()",
           stderr);
    return status;
  }

  SDL_QueryTexture(pNum_input_widget->widget_texture, NULL, NULL,
                   &pNum_input_widget->widget_pos.w,
                   &pNum_input_widget->widget_pos.h);

  return status;
}

Enum_StatusCodes
EditNumInputWidgetValue(Struct_NumInputWidget *pNum_input_widget,
                        SDL_Renderer *renderer, uint32_t new_val) {
  /*
  ? This could be optimized but even if world record typists,
  ? started to type at 10 widgets simultaneously, even ancient PCs would handle
  ? it with ease.
  */
  char temp[WIDGET_CHARACTER_LIMIT];
  /*
  Directly passing pNum_input_widget->widget_label_raw will cause it to become
  "". This is because we shouldn't snprintf to a string from the same string's
  value.
  */
  snprintf(temp, sizeof(temp), "%s", pNum_input_widget->widget_label_raw);

  return CreateNumInputWidget(
      pNum_input_widget, pNum_input_widget->widget_font, renderer,
      pNum_input_widget->widget_r, pNum_input_widget->widget_g,
      pNum_input_widget->widget_b, temp, new_val,
      pNum_input_widget->widget_pos.x, pNum_input_widget->widget_pos.y);
}

void FreeNumInputWidget(Struct_NumInputWidget *pNum_input_widget) {
  if (pNum_input_widget) {
    if (pNum_input_widget->widget_texture) {
      SDL_DestroyTexture(pNum_input_widget->widget_texture);
      pNum_input_widget->widget_texture = NULL;
    }
    // As the font could have global scope, we should not close it here.
    pNum_input_widget->widget_font = NULL;

    // Rest will be taken care as its stack memory.
  }
}

Enum_StatusCodes
InitSelectedColorState(Struct_SelectedColorState *pSelected_color_state,
                       TTF_Font *font, SDL_Renderer *renderer) {
  Enum_StatusCodes status = SUCCESS;
  pSelected_color_state->selected = NULL;
  pSelected_color_state->color_rect =
      (SDL_Rect){.x = 1075, .y = 30, .w = 50, .h = 50};

  status = CreateNumInputWidget(&pSelected_color_state->r, font, renderer,
                                DARK_REDDISH, "R: ", 0, 1075,
                                pSelected_color_state->color_rect.y +
                                    pSelected_color_state->color_rect.h + 10);
  status = CreateNumInputWidget(&pSelected_color_state->g, font, renderer,
                                DARK_GREENISH, "G: ", 0, 1075,
                                pSelected_color_state->r.widget_pos.y +
                                    pSelected_color_state->r.widget_pos.h + 10);
  status = CreateNumInputWidget(&pSelected_color_state->b, font, renderer,
                                DARK_BLUEISH, "B: ", 0, 1075,
                                pSelected_color_state->g.widget_pos.y +
                                    pSelected_color_state->g.widget_pos.h + 10);

  return status;
}

void ExitSelectedColorState(Struct_SelectedColorState *pSelected_color_state) {
  if (pSelected_color_state) {
    pSelected_color_state->selected = NULL;
    FreeNumInputWidget(&pSelected_color_state->r);
    FreeNumInputWidget(&pSelected_color_state->g);
    FreeNumInputWidget(&pSelected_color_state->b);
  }
}