#include "include/gfx.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FPS 60
#define FRAME_DELAY (1000 / MAX_FPS)
#define INPUT_DELAY 75

#define FILE_TO_WORK_ON "Demo.obj"

#define WINDOW_TITLE "Tile Editor"
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define FONT_SIZE 25
#define FONT_PATH "assets/JetBrainMono.ttf"

#define GRID_WIDTH 1000
#define GRID_HEIGHT 800
#define TILE_SIZE 50

#define REDDISH 255, 128, 128
#define GREENISH 128, 255, 128
#define BLUEISH 128, 128, 255
#define DARK_REDDISH 155, 28, 28
#define DARK_GREENISH 28, 155, 28
#define DARK_BLUEISH 28, 28, 155
#define BLACKISH 50, 50, 50
#define WHITISH 200, 200, 200

#define KNUTHS_X_MULTIPLIER 2654435761U
#define KNUTHS_Y_MULTIPLIER 2246822519U
#define HASH_BUCKET_SIZE 5000

#define MAX_LINE_SIZE 40
#define PERLINE_ATTR_COUNT 5
#define LINES_PER_RECT 6

#define SET_FLAG(var, flag) ((var) |= (flag))
#define CLEAR_FLAG(var, flag) ((var) &= ~(flag))
#define TOGGLE_FLAG(var, flag) ((var) ^= (flag))
#define HAS_FLAG(var, flag) (((var) & (flag)) != 0)

typedef enum Enum_StatusCodes {
  SUCCESS = 0,
  FAILURE = 1 << 0,
  // These will be along side the error to signify how the are handled.
  // Absence of these will automatically assume success and exit.
  LOW_SEVERITY_ERROR = 1 << 1,
  HIGH_SEVERITY_ERROR = 1 << 2,
  // Actual error codes.
  MEM_ALLOC_FAILURE = 1 << 3,
  INVALID_FUNCTION_INPUT = 1 << 4,
  INVALID_FILE_PATH = 1 << 5,
  DEPENDENCY_FAILURE = 1 << 6,
  UNEXPECTED_COMPUTED_RESULTS = 1 << 7,
  FILE_IO_ERROR = 1 << 8
} Enum_StatusCodes;

typedef enum Enum_Inputs {
  UP = 1,
  DOWN = 2,
  LEFT = 4,
  RIGHT = 8,
  BACKSPACE = 16,
  ENTER = 32,
  QUIT = 64,
  MSB = 128,
} Enum_Inputs;

typedef struct Struct_TileHashNode {
  int32_t x, y;
  uint8_t r, g, b;
  struct Struct_TileHashNode *next;
} Struct_TileNode;

typedef struct Struct_ColorState {
  uint8_t color_val;
  // The current info displayed only needs 3 bytes.
  // This is the color value display text that shall be rendered.
  char color_text[7];
  // Texture that has the rendered text.
  SDL_Texture *color_texture;
  // The position of the texture to go on the screen
  SDL_Rect color_rect;
} Struct_ColorState;

typedef struct Struct_ColorPickState {
  Struct_ColorState r, g, b;
  SDL_Rect color_pick_rect;
  Struct_ColorState *current_editing_state;
} Struct_ColorPickState;

//   ---------------------   //
//   Function Declaration.   //
//   ---------------------   //

//   ---------------------------   //
//   Loggers and Error Handlers.   //
//   ---------------------------   //

static void Logger(Enum_StatusCodes error_codes,
                   const char *(*extra_logs_callback)(void),
                   const char *extra_logs, FILE *output_stream);

//   -------------------------------   //
//   Utility functions of a hashmap.   //
//   -------------------------------   //
static uint32_t KnuthMultiplicativeHash(int32_t x, int32_t y);
static Enum_StatusCodes InitTileHashMap(Struct_TileNode ***pTile_hash_arr);
static void FreeTileHashMap(Struct_TileNode ***pTile_hash_arr);
static Enum_StatusCodes AddTileHashMapEntry(int32_t x, int32_t y, uint8_t r,
                                            uint8_t g, uint8_t b,
                                            Struct_TileNode **tile_hash_arr);
static Struct_TileNode *AccessTileHashMap(int32_t x, int32_t y,
                                          Struct_TileNode **tile_hash_arr);
static Enum_StatusCodes PopTileHashMapEntry(int32_t x, int32_t y,
                                            Struct_TileNode **tile_hash_arr);

//   ----------------------------   //
//   Data To File / File To Data.   //
//   ----------------------------   //

static Enum_StatusCodes DumpDataToFile(Struct_TileNode **tile_hash_arr,
                                       const char *file_path);
static Enum_StatusCodes ParseVDataLine(char *data_line,
                                       Struct_TileNode **tile_hash_arr);
static Enum_StatusCodes ParseFileToData(Struct_TileNode **tile_hash_arr,
                                        const char *file_path);

//   -------------------------------   //
//   Initialization and Destruction.   //
//   -------------------------------   //

static Enum_StatusCodes InitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
static void ExitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer);
static Enum_StatusCodes InitTTF(TTF_Font **pFont);
static void ExitTTF(TTF_Font **pFont);
static Enum_StatusCodes CreateTextTexture(SDL_Texture **pTexture,
                                          const char *text,
                                          SDL_Renderer *renderer,
                                          TTF_Font *font, uint8_t text_r,
                                          uint8_t text_g, uint8_t text_b);
static Enum_StatusCodes CreateColorState(Struct_ColorState *pColor_state,
                                         SDL_Renderer *renderer, TTF_Font *font,
                                         uint8_t color_val, char color_pick,
                                         int32_t x_pos, int32_t y_pos);
static Enum_StatusCodes CreateColorState(Struct_ColorState *pColor_state,
                                         SDL_Renderer *renderer, TTF_Font *font,
                                         uint8_t color_val, char color_pick,
                                         int32_t x_pos, int32_t y_pos);
static Enum_StatusCodes
InitColorPickState(Struct_ColorPickState *pColor_pick_state,
                   SDL_Renderer *renderer, TTF_Font *font);
static void ExitColorPickState(Struct_ColorPickState *pColor_pick_state);

//   --------------------   //
//   Rendering utilities.   //
//   --------------------   //

static void RenderGrid(SDL_Renderer *renderer, Struct_TileNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset);
static void RenderColorPick(SDL_Renderer *renderer,
                            Struct_ColorPickState *pColor_pick_state);
static Enum_StatusCodes Render(SDL_Renderer *renderer,
                               Struct_TileNode **tile_hash_arr,
                               int32_t move_x_offset, int32_t move_y_offset,
                               Struct_ColorPickState *pColor_pick_state);

//   ---------------------------------   //
//   Logic and manipulation functions.   //
//   ---------------------------------   //

static void GetGridPos(uint32_t screen_x, uint32_t screen_y, uint32_t *pGrid_x,
                       uint32_t *pGrid_y);
static Enum_StatusCodes CheckRectMouseCollision(SDL_Rect *pRect,
                                                int32_t mouse_click_x,
                                                int32_t mouse_click_y);
static Enum_Inputs GetInput(uint32_t *pMouse_click_x, uint32_t *pMouse_click_y);

//   -----------------------   //
//   State handling utility.   //
//   -----------------------   //

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileNode **tile_hash_arr,
                             Struct_ColorPickState *pColor_pick_state,
                             int32_t move_x_offset, int32_t move_y_offset);
static void HandleUtilClicks(uint32_t *pMouse_click_x, uint32_t *pMouse_click_y,
                             Struct_ColorPickState *pColor_pick_state,
                             Enum_Inputs input_flags, SDL_Renderer *renderer,
                             TTF_Font *font);
static void HandleGridMoving(uint32_t input_flags, int32_t *pMove_x_offset,
                             int32_t *pMove_y_offset);
static Enum_StatusCodes
HandleState(Enum_Inputs input_flags, uint32_t *pMouse_click_x,
            uint32_t *pMouse_click_y, SDL_Renderer *renderer, TTF_Font *font,
            Struct_TileNode **tile_hash_arr,
            Struct_ColorPickState *pColor_pick_state, int32_t *pMove_x_offset,
            int32_t *pMove_y_offset, uint32_t *pCurrent_time);

//   -------------------   //
//   Main app functions.   //
//   -------------------   //

static Enum_StatusCodes InitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                                Struct_TileNode ***pTile_hash_arr,
                                TTF_Font **pFont,
                                Struct_ColorPickState *pColor_pick_state);
static void Loop(SDL_Renderer *renderer, Struct_TileNode **tile_hash_arr,
                 TTF_Font *font, Struct_ColorPickState *pColor_pick_state);
static void ExitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                    Struct_TileNode ***pTile_hash_arr, TTF_Font **pFont,
                    Struct_ColorPickState *pColor_pick_state);

//   ---------------------   //
//   Function Definations.   //
//   ---------------------   //

//   ---------------------------   //
//   Loggers and Error Handlers.   //
//   ---------------------------   //

static void Logger(Enum_StatusCodes error_codes,
                   const char *(*extra_logs_callback)(void),
                   const char *extra_logs, FILE *output_stream) {
  const char *callback_logs = "";
  if (extra_logs_callback) {
    callback_logs = extra_logs_callback();
    if (!callback_logs) {
      callback_logs = "";
    }
  }

  if (!extra_logs) {
    extra_logs = "";
  }

  if (!output_stream) {
    output_stream = stderr;
  }

  fprintf(output_stream, "\nLOGS:\n");
  fprintf(output_stream, "  FUNCTION CALLBACK LOGS:\n    %s\n", callback_logs);
  fprintf(output_stream, "  EXTRA LOGS:\n    %s\n", extra_logs);
  fprintf(output_stream, "  STATUS CODE'S LOGS:\n    ");

  if (error_codes) {
    if (HAS_FLAG(error_codes, HIGH_SEVERITY_ERROR)) {
      fprintf(output_stream, "    [ERROR]: ");
    } else if (HAS_FLAG(error_codes, LOW_SEVERITY_ERROR)) {
      fprintf(output_stream, "    [WARNING]: ");
    }
    /*
    In the current project scope, encountering multiple errors per Logger call
    is highly unlikely. Therefore, we are not implementing multi-error handling
    for now. This can be reconsidered if requirements change.
    */
    if (HAS_FLAG(error_codes, MEM_ALLOC_FAILURE)) {
      fprintf(output_stream,
              "Unable to allocate memory, malloc/calloc/realloc failure.\n");
    } else if (HAS_FLAG(error_codes, INVALID_FUNCTION_INPUT)) {
      fprintf(output_stream, "Invalid/NULL inputs passed to a functions.\n");
    } else if (HAS_FLAG(error_codes, INVALID_FILE_PATH)) {
      fprintf(output_stream, "Invalid file path provided.\n");
    } else if (HAS_FLAG(error_codes, DEPENDENCY_FAILURE)) {
      fprintf(output_stream,
              "Some internal dependency failed to give desired results.\n");
    } else if (HAS_FLAG(error_codes, UNEXPECTED_COMPUTED_RESULTS)) {
      fprintf(
          output_stream,
          "Produced unexpected results. Most likely invalid data provided.\n");
    }
  }
}

//   -------------------------------   //
//   Utility functions of a hashmap.   //
//   -------------------------------   //

static uint32_t KnuthMultiplicativeHash(int32_t x, int32_t y) {
  uint32_t ux = (uint32_t)x * KNUTHS_X_MULTIPLIER;
  uint32_t uy = (uint32_t)y * KNUTHS_Y_MULTIPLIER;
  uint32_t raw_hash =
      (ux ^ (uy >> 16) ^ (uy << 13) ^ (x >> 5) ^ (y << 7)); // Extra mixing

  return raw_hash % HASH_BUCKET_SIZE;
}

static Enum_StatusCodes InitTileHashMap(Struct_TileNode ***pTile_hash_arr) {
  if (!pTile_hash_arr) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by InitTileHashMap()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }
  *pTile_hash_arr = calloc(HASH_BUCKET_SIZE, sizeof(Struct_TileNode *));
  if (!(*pTile_hash_arr)) {
    Logger(MEM_ALLOC_FAILURE | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by InitTileHashMap()", stderr);
    return MEM_ALLOC_FAILURE | HIGH_SEVERITY_ERROR;
  }

  return SUCCESS;
}

static void FreeTileHashMap(Struct_TileNode ***pTile_hash_arr) {
  if (pTile_hash_arr && *pTile_hash_arr) {
    Struct_TileNode *temp;

    for (int i = 0; i < HASH_BUCKET_SIZE; i++) {
      while ((*pTile_hash_arr)[i]) {
        temp = (*pTile_hash_arr)[i]->next;
        free((*pTile_hash_arr)[i]);
        (*pTile_hash_arr)[i] = temp;
      }
    }
    free(*pTile_hash_arr);
    (*pTile_hash_arr) = NULL;
  }
}

static Enum_StatusCodes AddTileHashMapEntry(int32_t x, int32_t y, uint8_t r,
                                            uint8_t g, uint8_t b,
                                            Struct_TileNode **tile_hash_arr) {
  if (!tile_hash_arr) {
    Logger(INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR, NULL,
           "Error produced by AddTileHashMapEntry()", stderr);
    return INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR;
  }

  Struct_TileNode *node = malloc(sizeof(Struct_TileNode));
  if (!node) {
    Logger(MEM_ALLOC_FAILURE | LOW_SEVERITY_ERROR, NULL,
           "Error produced by AddTileHashMapEntry()", stderr);
    return MEM_ALLOC_FAILURE | LOW_SEVERITY_ERROR;
  }
  node->x = x;
  node->y = y;
  node->r = r;
  node->g = g;
  node->b = b;

  uint32_t index = KnuthMultiplicativeHash(x, y);

  node->next = tile_hash_arr[index];
  tile_hash_arr[index] = node;

  return SUCCESS;
}

static Struct_TileNode *AccessTileHashMap(int32_t x, int32_t y,
                                          Struct_TileNode **tile_hash_arr) {
  if (!tile_hash_arr) {
    Logger(INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR, NULL,
           "Error produced by AccessTileHashMap()", stderr);
    return NULL;
  }

  uint32_t index = KnuthMultiplicativeHash(x, y);
  Struct_TileNode *curr = tile_hash_arr[index];

  while (curr) {
    if (curr->x == x && curr->y == y) {
      return curr;
    }
    curr = curr->next;
  }

  return NULL;
}

static Enum_StatusCodes PopTileHashMapEntry(int32_t x, int32_t y,
                                            Struct_TileNode **tile_hash_arr) {
  if (!tile_hash_arr) {
    Logger(INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR, NULL,
           "Error produced by PopTileHashMapEntry()", stderr);
    return INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR;
  }

  uint32_t index = KnuthMultiplicativeHash(x, y);
  Struct_TileNode *curr = tile_hash_arr[index];
  Struct_TileNode *prev = NULL;

  while (curr) {
    if (curr->x == x && curr->y == y) {
      if (!prev) {
        tile_hash_arr[index] = curr->next;
      } else {
        prev->next = curr->next;
      }
      free(curr);
      return SUCCESS;
    }
    prev = curr;
    curr = curr->next;
  }

  return FAILURE;
}

//   ----------------------------   //
//   Data To File / File To Data.   //
//   ----------------------------   //

static Enum_StatusCodes DumpDataToFile(Struct_TileNode **tile_hash_arr,
                                       const char *file_path) {
  if (!tile_hash_arr || !file_path) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by DumpDataToFile()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }

  FILE *file = fopen(file_path, "w");
  if (!file) {
    Logger(INVALID_FILE_PATH | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by DumpDataToFile()", stderr);
    return INVALID_FILE_PATH | HIGH_SEVERITY_ERROR;
  }

  int32_t vert_c = 0;
  for (int i = 0; i < HASH_BUCKET_SIZE; i++) {
    Struct_TileNode *curr = tile_hash_arr[i];
    while (curr) {
      fprintf(file,
              "\nv %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "v %d %d %d %d %d\n"
              "i %d %d %d\n"
              "i %d %d %d\n",
              curr->x, curr->y, curr->r, curr->g, curr->b, curr->x + TILE_SIZE,
              curr->y, curr->r, curr->g, curr->b, curr->x, curr->y + TILE_SIZE,
              curr->r, curr->g, curr->b, curr->x + TILE_SIZE,
              curr->y + TILE_SIZE, curr->r, curr->g, curr->b, vert_c + 0,
              vert_c + 1, vert_c + 3, vert_c + 0, vert_c + 2, vert_c + 3);
      curr = curr->next;
      vert_c += 4;
    }
  }
  fclose(file);

  return SUCCESS;
}

static Enum_StatusCodes ParseVDataLine(char *data_line,
                                       Struct_TileNode **tile_hash_arr) {
  char *token = strtok(&data_line[2], " \n");
  uint32_t i = 0;
  int32_t x, y;
  uint32_t r, g, b;

  while (token && i < PERLINE_ATTR_COUNT) {
    char *end_ptr;
    int32_t data_token = strtol(token, &end_ptr, 10);
    if (end_ptr[0] != '\0' || end_ptr == token) {
      fprintf(stderr, "Invalid data in given vertex line.\n");
      return 0;
    }
    switch (i) {
    case 0:
      x = data_token;
      break;
    case 1:
      y = data_token;
      break;
    case 2:
      r = data_token;
      break;
    case 3:
      g = data_token;
      break;
    case 4:
      b = data_token;
      break;
    default:
      break;
    }
    token = strtok(NULL, " \n");
    i++;
  }
  if (i < PERLINE_ATTR_COUNT) {
    Logger(UNEXPECTED_COMPUTED_RESULTS | LOW_SEVERITY_ERROR, NULL,
           "Error produced by ParseVDataLine()", stderr);
    return UNEXPECTED_COMPUTED_RESULTS | LOW_SEVERITY_ERROR;
  }

  return AddTileHashMapEntry(x, y, r, g, b, tile_hash_arr);
}

static Enum_StatusCodes ParseFileToData(Struct_TileNode **tile_hash_arr,
                                        const char *file_path) {
  if (!tile_hash_arr || !file_path) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by ParseFileToData()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }

  FILE *file = fopen(file_path, "r");
  if (!file) {
    Logger(INVALID_FILE_PATH | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by ParseFileToData()", stderr);
    return INVALID_FILE_PATH | HIGH_SEVERITY_ERROR;
  }

  char buffer[MAX_LINE_SIZE];
  Enum_StatusCodes status_codes;

  while (fgets(buffer, sizeof(buffer), file)) {
    if (buffer[strlen(buffer) - 1] != '\n' && !feof(file)) {
      // Digesting the entire line to ignore.
      char temp;
      while ((temp = fgetc(file)) != '\n' && temp != EOF)
        ;
      continue;
    } else if (buffer[0] == 'v' && buffer[1] == ' ') {
      if ((status_codes = ParseVDataLine(buffer, tile_hash_arr)) != SUCCESS) {
        fclose(file);
        return status_codes;
      }
      // Digesting vertices and indices that makeup the rect and just directly
      // building it here. Assuming data correctness.
      for (int i = 0; i < LINES_PER_RECT; i++) {
        if (!fgets(buffer, MAX_LINE_SIZE, file) && !feof(file)) {
          Logger(FILE_IO_ERROR | HIGH_SEVERITY_ERROR, NULL,
                 "Error produced by ParseFileToData()", stderr);
          fclose(file);
          return FILE_IO_ERROR | HIGH_SEVERITY_ERROR;
        }
      }
    }
  }
  fclose(file);

  return SUCCESS;
}

//   -------------------------------   //
//   Initialization and Destruction.   //
//   -------------------------------   //

static Enum_StatusCodes InitSDL(SDL_Window **pWindow,
                                SDL_Renderer **pRenderer) {
  if (!pWindow || !pRenderer) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by InitSDL()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    Logger(DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR, SDL_GetError,
           "Error produced by InitSDL()", stderr);
    return DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
  }

  *pWindow = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
  if (!pWindow) {
    Logger(DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR, SDL_GetError,
           "Error produced by InitSDL()", stderr);
    return DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
  }

  *pRenderer = SDL_CreateRenderer(*pWindow, -1, SDL_RENDERER_ACCELERATED);
  if (!pRenderer) {
    SDL_DestroyWindow(*pWindow);
    Logger(DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR, SDL_GetError,
           "Error produced by InitSDL()", stderr);
    return DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
  }

  return SUCCESS;
}

static void ExitSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer) {
  if (pWindow && *pWindow) {
    SDL_DestroyWindow(*pWindow);
    *pWindow = NULL;
  }
  if (pRenderer && *pRenderer) {
    SDL_DestroyRenderer(*pRenderer);
    *pRenderer = NULL;
  }
  SDL_Quit();
}

static Enum_StatusCodes InitTTF(TTF_Font **pFont) {
  if (!pFont) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by InitTTF()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }
  if (TTF_Init()) {
    Logger(DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR, TTF_GetError,
           "Error produced by InitTTF()", stderr);
    return DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
  }

  *pFont = TTF_OpenFont(FONT_PATH, FONT_SIZE);
  if (!(*pFont)) {
    Logger(DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR, TTF_GetError,
           "Error produced by InitTTF()", stderr);
    return DEPENDENCY_FAILURE | HIGH_SEVERITY_ERROR;
  }

  return SUCCESS;
}

static void ExitTTF(TTF_Font **pFont) {
  if (pFont && *pFont) {
    TTF_CloseFont(*pFont);
  }

  TTF_Quit();
}

static Enum_StatusCodes CreateTextTexture(SDL_Texture **pTexture,
                                          const char *text,
                                          SDL_Renderer *renderer,
                                          TTF_Font *font, uint8_t text_r,
                                          uint8_t text_g, uint8_t text_b) {
  SDL_Colour color = {.r = text_r, .g = text_g, .b = text_b, .a = 255};
  SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, color);
  if (!text_surface) {
    Logger(DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR, SDL_GetError,
           "Error produced by CreateTextTexture()", stderr);
    return DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR;
  }

  *pTexture = SDL_CreateTextureFromSurface(renderer, text_surface);

  // Freeing is imp, this is done here to save 1 line of code.
  SDL_FreeSurface(text_surface);

  if (!(*pTexture)) {
    Logger(DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR, SDL_GetError,
           "Error produced by CreateTextTexture()", stderr);
    return DEPENDENCY_FAILURE | LOW_SEVERITY_ERROR;
  }

  return SUCCESS;
}

static Enum_StatusCodes CreateColorState(Struct_ColorState *pColor_state,
                                         SDL_Renderer *renderer, TTF_Font *font,
                                         uint8_t color_val, char color_pick,
                                         int32_t x_pos, int32_t y_pos) {
  if ((color_pick != 'R' && color_pick != 'G' && color_pick != 'B')) {
    Logger(INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR, NULL,
           "Error produced by CreateColorState()", stderr);
    return INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR;
  }
  pColor_state->color_val = color_val;
  snprintf(pColor_state->color_text, sizeof(pColor_state->color_text), "%c: %d",
           color_pick, color_val);
  /*
  As the texture creation could fail, but is not a major point to exit the
  game, the error code will be returned but will not be a point of
  cancellation. Obviously the final render will be buggy, but we shall be able
  to correct is by a restart.
  */
  Enum_StatusCodes status_codes = SUCCESS;
  if (color_pick == 'R') {
    status_codes = CreateTextTexture(&pColor_state->color_texture,
                                     pColor_state->color_text, renderer, font,
                                     DARK_REDDISH);
  } else if (color_pick == 'G') {
    status_codes = CreateTextTexture(&pColor_state->color_texture,
                                     pColor_state->color_text, renderer, font,
                                     DARK_GREENISH);
  } else if (color_pick == 'B') {
    status_codes = CreateTextTexture(&pColor_state->color_texture,
                                     pColor_state->color_text, renderer, font,
                                     DARK_BLUEISH);
  }

  if (pColor_state->color_texture) {
    pColor_state->color_rect = (SDL_Rect){.x = x_pos, .y = y_pos};
    SDL_QueryTexture(pColor_state->color_texture, NULL, NULL,
                     &pColor_state->color_rect.w, &pColor_state->color_rect.h);
  }

  return status_codes;
}

static Enum_StatusCodes
InitColorPickState(Struct_ColorPickState *pColor_pick_state,
                   SDL_Renderer *renderer, TTF_Font *font) {
  if (!pColor_pick_state || !renderer || !font) {
    Logger(INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR, NULL,
           "Error produced by InitColorPickState()", stderr);
    return INVALID_FUNCTION_INPUT | LOW_SEVERITY_ERROR;
  }

  pColor_pick_state->current_editing_state = NULL;
  pColor_pick_state->color_pick_rect =
      (SDL_Rect){.x = 1075, .y = 50, .w = 50, .h = 50};

  Enum_StatusCodes status_codes = SUCCESS;
  /*
  We use status codes rather than just return call1() || call2() || call3(),
  as if we return, as soon as there is a failure, we return rather than going
  forward and trying to create other ones.
  */
  status_codes |=
      CreateColorState(&pColor_pick_state->r, renderer, font, 0, 'R', 1025,
                       pColor_pick_state->color_pick_rect.y +
                           pColor_pick_state->color_pick_rect.h + 5);
  status_codes |=
      CreateColorState(&pColor_pick_state->g, renderer, font, 0, 'G', 1025,
                       pColor_pick_state->r.color_rect.y +
                           pColor_pick_state->r.color_rect.h + 5);
  status_codes |=
      CreateColorState(&pColor_pick_state->b, renderer, font, 0, 'B', 1025,
                       pColor_pick_state->g.color_rect.y +
                           pColor_pick_state->g.color_rect.h + 5);
  return status_codes;
}

static void ExitColorPickState(Struct_ColorPickState *pColor_pick_state) {
  if (pColor_pick_state) {
    if (pColor_pick_state->r.color_texture) {
      SDL_DestroyTexture(pColor_pick_state->r.color_texture);
    }
    if (pColor_pick_state->g.color_texture) {
      SDL_DestroyTexture(pColor_pick_state->g.color_texture);
    }
    if (pColor_pick_state->b.color_texture) {
      SDL_DestroyTexture(pColor_pick_state->b.color_texture);
    }
    pColor_pick_state->current_editing_state = NULL;
  }
}

//   --------------------   //
//   Rendering utilities.   //
//   --------------------   //

static void RenderGrid(SDL_Renderer *renderer, Struct_TileNode **tile_hash_arr,
                       int32_t move_x_offset, int32_t move_y_offset) {
  SDL_Rect rect = {.w = TILE_SIZE, .h = TILE_SIZE};
  Struct_TileNode *temp;

  for (int i = 0; i < GRID_HEIGHT; i += TILE_SIZE) {
    for (int j = 0; j < GRID_WIDTH; j += TILE_SIZE) {
      rect.y = i;
      rect.x = j;
      if ((temp = AccessTileHashMap(j + move_x_offset, i + move_y_offset,
                                    tile_hash_arr))) {
        SDL_SetRenderDrawColor(renderer, temp->r, temp->g, temp->b, 255);
        SDL_RenderFillRect(renderer, &rect);
      } else {
        SDL_SetRenderDrawColor(renderer, BLACKISH, 255);
        SDL_RenderDrawRect(renderer, &rect);
      }
    }
  }
}

static void RenderColorPick(SDL_Renderer *renderer,
                            Struct_ColorPickState *pColor_pick_state) {
  SDL_SetRenderDrawColor(renderer, pColor_pick_state->r.color_val,
                         pColor_pick_state->g.color_val,
                         pColor_pick_state->b.color_val, 255);
  SDL_RenderFillRect(renderer, &pColor_pick_state->color_pick_rect);

  if (pColor_pick_state->r.color_texture) {
    SDL_RenderCopy(renderer, pColor_pick_state->r.color_texture, NULL,
                   &pColor_pick_state->r.color_rect);
  }
  if (pColor_pick_state->g.color_texture) {
    SDL_RenderCopy(renderer, pColor_pick_state->g.color_texture, NULL,
                   &pColor_pick_state->g.color_rect);
  }
  if (pColor_pick_state->b.color_texture) {
    SDL_RenderCopy(renderer, pColor_pick_state->b.color_texture, NULL,
                   &pColor_pick_state->b.color_rect);
  }
}

static Enum_StatusCodes Render(SDL_Renderer *renderer,
                               Struct_TileNode **tile_hash_arr,
                               int32_t move_x_offset, int32_t move_y_offset,
                               Struct_ColorPickState *pColor_pick_state) {
  if (!renderer || !tile_hash_arr || !pColor_pick_state) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by Render()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }

  SDL_RenderClear(renderer);
  RenderGrid(renderer, tile_hash_arr, move_x_offset, move_y_offset);
  RenderColorPick(renderer, pColor_pick_state);
  SDL_SetRenderDrawColor(renderer, WHITISH, 255);
  SDL_RenderPresent(renderer);

  return SUCCESS;
}

//   ---------------------------------   //
//   Logic and manipulation functions.   //
//   ---------------------------------   //

static void GetGridPos(uint32_t screen_x, uint32_t screen_y, uint32_t *pGrid_x,
                       uint32_t *pGrid_y) {
  *pGrid_x = (screen_x / TILE_SIZE) * TILE_SIZE;
  *pGrid_y = (screen_y / TILE_SIZE) * TILE_SIZE;
}

static Enum_StatusCodes CheckRectMouseCollision(SDL_Rect *pRect,
                                                int32_t mouse_click_x,
                                                int32_t mouse_click_y) {
  if (mouse_click_x >= (pRect)->x && mouse_click_x <= (pRect->x + pRect->w) &&
      mouse_click_y >= (pRect)->y && mouse_click_y <= (pRect->y + pRect->h)) {
    return SUCCESS;
  }

  return FAILURE;
}

static Enum_Inputs GetInput(uint32_t *pMouse_click_x,
                            uint32_t *pMouse_click_y) {
  Enum_Inputs input_flags = 0;
  SDL_Event event;

  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      SET_FLAG(input_flags, QUIT);
      return input_flags;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      SET_FLAG(input_flags, MSB);
      *pMouse_click_x = event.button.x;
      *pMouse_click_y = event.button.y;
    }
    if (event.type == SDL_TEXTINPUT && isdigit(event.text.text[0])) {
      // Adding the value of the digit pressed to the leftmost byte of the
      // input_flag. This is so that we can easily edit in app rgb.
      SET_FLAG(input_flags, event.text.text[0] << 24);
    }
    if (event.type == SDL_KEYDOWN) {
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
    }
  }

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

  // If opposite direction movement flags exist, they cancel out.
  if (HAS_FLAG(input_flags, UP) && HAS_FLAG(input_flags, DOWN)) {
    CLEAR_FLAG(input_flags, UP);
    CLEAR_FLAG(input_flags, DOWN);
  }
  if (HAS_FLAG(input_flags, LEFT) && HAS_FLAG(input_flags, RIGHT)) {
    CLEAR_FLAG(input_flags, LEFT);
    CLEAR_FLAG(input_flags, RIGHT);
  }

  return input_flags;
}

//   -----------------------   //
//   State handling utility.   //
//   -----------------------   //

static void HandleTileClicks(uint32_t grid_x, uint32_t grid_y,
                             Struct_TileNode **tile_hash_arr,
                             Struct_ColorPickState *pColor_pick_state,
                             int32_t move_x_offset, int32_t move_y_offset) {
  if (PopTileHashMapEntry(grid_x + move_x_offset, grid_y + move_y_offset,
                          tile_hash_arr) == SUCCESS) {
    return;
  } else {
    AddTileHashMapEntry(grid_x + move_x_offset, grid_y + move_y_offset,
                        pColor_pick_state->r.color_val,
                        pColor_pick_state->g.color_val,
                        pColor_pick_state->b.color_val, tile_hash_arr);
  }
}

static void HandleUtilClicks(uint32_t *pMouse_click_x, uint32_t *pMouse_click_y,
                             Struct_ColorPickState *pColor_pick_state,
                             Enum_Inputs input_flags, SDL_Renderer *renderer,
                             TTF_Font *font) {
  // *Assigning what value to edit.
  if (pColor_pick_state->current_editing_state &&
      CheckRectMouseCollision(
          &pColor_pick_state->current_editing_state->color_rect,
          *pMouse_click_x, *pMouse_click_y) == SUCCESS) {
    // No new clicks
  } else if (CheckRectMouseCollision(&pColor_pick_state->r.color_rect,
                                     *pMouse_click_x,
                                     *pMouse_click_y) == SUCCESS) {
    pColor_pick_state->current_editing_state = &pColor_pick_state->r;
    *pMouse_click_x = pColor_pick_state->current_editing_state->color_rect.x;
    *pMouse_click_y = pColor_pick_state->current_editing_state->color_rect.y;
  } else if (CheckRectMouseCollision(&pColor_pick_state->g.color_rect,
                                     *pMouse_click_x,
                                     *pMouse_click_y) == SUCCESS) {
    pColor_pick_state->current_editing_state = &pColor_pick_state->g;
    *pMouse_click_x = pColor_pick_state->current_editing_state->color_rect.x;
    *pMouse_click_y = pColor_pick_state->current_editing_state->color_rect.y;
  } else if (CheckRectMouseCollision(&pColor_pick_state->b.color_rect,
                                     *pMouse_click_x,
                                     *pMouse_click_y) == SUCCESS) {
    pColor_pick_state->current_editing_state = &pColor_pick_state->b;
    *pMouse_click_x = pColor_pick_state->current_editing_state->color_rect.x;
    *pMouse_click_y = pColor_pick_state->current_editing_state->color_rect.y;
  } else if (HAS_FLAG(input_flags, ENTER)) {
    pColor_pick_state->current_editing_state = NULL;
  } else {
    pColor_pick_state->current_editing_state = NULL;
  }

  // *Editing the assigned value or if nothing can be performed, exiting
  char color_digit_input[2];
  color_digit_input[0] = (input_flags >> 24);
  color_digit_input[1] = '\0';
  if (!pColor_pick_state->current_editing_state ||
      (color_digit_input[0] && HAS_FLAG(input_flags, BACKSPACE))) {
    /*
    Either there is nothing to do or both actions we can do cancel each other
    out so its better to quit.
    */
    return;
  }

  char color_state_pick =
      pColor_pick_state->current_editing_state->color_text[0];
  uint32_t new_color_val = pColor_pick_state->current_editing_state->color_val;

  if (color_digit_input[0]) {
    new_color_val *= 10;
    new_color_val += atoi(color_digit_input);
  } else if (HAS_FLAG(input_flags, BACKSPACE)) {
    new_color_val /= 10;
  }
  if (new_color_val > 255) {
    new_color_val = pColor_pick_state->current_editing_state->color_val;
  }
  SDL_DestroyTexture(pColor_pick_state->current_editing_state->color_texture);
  CreateColorState(pColor_pick_state->current_editing_state, renderer, font,
                   new_color_val, color_state_pick,
                   pColor_pick_state->current_editing_state->color_rect.x,
                   pColor_pick_state->current_editing_state->color_rect.y);
}

static void HandleGridMoving(uint32_t input_flags, int32_t *pMove_x_offset,
                             int32_t *pMove_y_offset) {
  if (HAS_FLAG(input_flags, UP)) {
    *pMove_y_offset -= TILE_SIZE;
  } else if (HAS_FLAG(input_flags, DOWN)) {
    *pMove_y_offset += TILE_SIZE;
  }
  if (HAS_FLAG(input_flags, LEFT)) {
    *pMove_x_offset -= TILE_SIZE;
  } else if (HAS_FLAG(input_flags, RIGHT)) {
    *pMove_x_offset += TILE_SIZE;
  }
}

static Enum_StatusCodes
HandleState(Enum_Inputs input_flags, uint32_t *pMouse_click_x,
            uint32_t *pMouse_click_y, SDL_Renderer *renderer, TTF_Font *font,
            Struct_TileNode **tile_hash_arr,
            Struct_ColorPickState *pColor_pick_state, int32_t *pMove_x_offset,
            int32_t *pMove_y_offset, uint32_t *pCurrent_time) {
  if (!pMouse_click_x || !pMouse_click_y || !renderer || !font ||
      !tile_hash_arr || !pColor_pick_state || !pMove_x_offset ||
      !pMove_y_offset || !pCurrent_time) {
    Logger(INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR, NULL,
           "Error produced by HandleState()", stderr);
    return INVALID_FUNCTION_INPUT | HIGH_SEVERITY_ERROR;
  }

  if (HAS_FLAG(input_flags, MSB) && *pMouse_click_x <= GRID_WIDTH) {
    GetGridPos(*pMouse_click_x, *pMouse_click_y, pMouse_click_x,
               pMouse_click_y);
    HandleTileClicks(*pMouse_click_x, *pMouse_click_y, tile_hash_arr,
                     pColor_pick_state, *pMove_x_offset, *pMove_y_offset);
  } else if ((HAS_FLAG(input_flags, MSB) && *pMouse_click_x > GRID_WIDTH) ||
             pColor_pick_state->current_editing_state) {
    /*
    This will be executed if the user clicked on valid spots or if an edit is
    currently going on.
    If the later condition is true and we are already editing. The mouse pos
    would have been the same as when the user clicked on the designated
    position to edit. So it won't cause any undesired side effects
    */
    HandleUtilClicks(pMouse_click_x, pMouse_click_y, pColor_pick_state,
                     input_flags, renderer, font);
  }
  // This means we are not currently editing rgb and input delay is covered.
  if (!pColor_pick_state->current_editing_state &&
      SDL_GetTicks() - *pCurrent_time >= INPUT_DELAY) {
    *pCurrent_time = SDL_GetTicks();
    HandleGridMoving(input_flags, pMove_x_offset, pMove_y_offset);
  }

  return SUCCESS;
}

//   -------------------   //
//   Main app functions.   //
//   -------------------   //

static Enum_StatusCodes InitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                                Struct_TileNode ***pTile_hash_arr,
                                TTF_Font **pFont,
                                Struct_ColorPickState *pColor_pick_state) {
  if (InitSDL(pWindow, pRenderer) != SUCCESS ||
      InitTileHashMap(pTile_hash_arr) != SUCCESS || InitTTF(pFont) != SUCCESS) {
    return FAILURE;
  }
  if (ParseFileToData(*pTile_hash_arr, FILE_TO_WORK_ON) != SUCCESS) {
    return FAILURE;
  }

  InitColorPickState(pColor_pick_state, *pRenderer, *pFont);

  return SUCCESS;
}

static void Loop(SDL_Renderer *renderer, Struct_TileNode **tile_hash_arr,
                 TTF_Font *font, Struct_ColorPickState *pColor_pick_state) {
  uint32_t mouse_click_x = 0, mouse_click_y = 0;
  int32_t move_x_offset = 0, move_y_offset = 0;

  uint32_t current_time = SDL_GetTicks();
  Enum_Inputs input_flags = 0;

  /*
  We need to limit fps as this app is doesn't need high fps performance.

  But limiting fps also limits the input polling. This makes the user inputs
  have huge lags.

  So we only limit fps in the rendering and input processing phase. But poll
  input at full speed.

  We only limit input processing of the grid movement and not the mouse
  clicks.

  This is because a mouse click can't be continuously polled as the button
  need to be clicked repeatedly to be polled.

  We need to input delay movement, as it is deigned to grab keyboard state
  that will continuously be polled each frame without reclcking.
  */

  while (1) {
    input_flags = GetInput(&mouse_click_x, &mouse_click_y);
    if (HAS_FLAG(input_flags, QUIT)) {
      return;
    }
    if (HandleState(input_flags, &mouse_click_x, &mouse_click_y, renderer, font,
                    tile_hash_arr, pColor_pick_state, &move_x_offset,
                    &move_y_offset, &current_time) != SUCCESS) {
      return;
    }
    if (SDL_GetTicks() - current_time >= FRAME_DELAY) {
      if (Render(renderer, tile_hash_arr, move_x_offset, move_y_offset,
                 pColor_pick_state)) {
        return;
      }
    }
  }
}

static void ExitApp(SDL_Window **pWindow, SDL_Renderer **pRenderer,
                    Struct_TileNode ***pTile_hash_arr, TTF_Font **pFont,
                    Struct_ColorPickState *pColor_pick_state) {
  // If dumping fails, its way before the file was even opend, so no data loss.
  DumpDataToFile(*pTile_hash_arr, FILE_TO_WORK_ON);
  FreeTileHashMap(pTile_hash_arr);
  ExitColorPickState(pColor_pick_state);
  ExitTTF(pFont);
  ExitSDL(pWindow, pRenderer);
}

int32_t main(void) {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  Struct_TileNode **tile_hash_arr = NULL;
  TTF_Font *font = NULL;
  Struct_ColorPickState state;
  Struct_NumInputWidget actual_tile_size;

  if (InitApp(&window, &renderer, &tile_hash_arr, &font, &state) == SUCCESS) {
    Loop(renderer, tile_hash_arr, font, &state);
  }
  ExitApp(&window, &renderer, &tile_hash_arr, &font, &state);

  return 0;
}

/*
TODO: IMPORTANT, make a general way to edit states, like R, G, B or any other to
come in the future UI elements: Consider a widget or component-based system if
it grows

TODO: Add comments to each function that expects the caller to give valid

TODO: Add a UI state to select the tile size, also add zooming in/out.

Potential refactor: input grabber uses 1 byte to store the digit input's char
when editing rgb. Modify it to use 4 bits to store actual int value ranging
from 0-9. As we need to convert it to int regardless as the digit char is
useless to us.

Refactor code.
*/
