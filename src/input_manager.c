#include "../include/input_manager.h"
#include <SDL2/SDL_events.h>

Enum_Inputs GetInput(uint32_t *pRecorded_mouse_click_x,
                     uint32_t *pRecorded_mouse_click_y) {
  Enum_Inputs input_flags = 0;
  SDL_Event event;

  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      SET_FLAG(input_flags, QUIT);
      return input_flags;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      SET_FLAG(input_flags, MSB);
      *pRecorded_mouse_click_x = event.button.x;
      *pRecorded_mouse_click_y = event.button.y;
    } else if (event.type == SDL_KEYDOWN) {
      /*
      Inputs that we want the user to press each time will not be checked for
      in the keyboard state down below
      */
      if (event.key.keysym.sym == SDLK_BACKSPACE) {
        SET_FLAG(input_flags, BACKSPACE);
      }
      if (event.key.keysym.sym == SDLK_RETURN) {
        SET_FLAG(input_flags, ENTER);
      }
    } else if (event.type == SDL_TEXTINPUT) {
      SET_FLAG(input_flags, event.text.text[0] << INPUT_CHAR_BITMASK);
    }
  }
  /*
  We have to use states for things like movement keys. To provided continuous
  movement by holding the key.
  */
  const uint8_t *state = SDL_GetKeyboardState(NULL);

  if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {
    SET_FLAG(input_flags, UP);
  }
  if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
    SET_FLAG(input_flags, DOWN);
  }
  if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
    SET_FLAG(input_flags, LEFT);
  }
  if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
    SET_FLAG(input_flags, RIGHT);
  }

  return input_flags;
}