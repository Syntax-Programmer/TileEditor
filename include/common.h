#pragma once

#include <stdint.h>
#include <stdio.h>

#define APP_WIDTH 1200
#define APP_HEIGHT 800
#define APP_TITLE "TileEditor"
#define APP_FONT "assets/JetBrainMono.ttf"
#define FONT_SIZE 20

#define GRID_WIDTH 800
#define GRID_HEIGHT 800
#define TILE_SIZE 50

#define SET_FLAG(var, flag) ((var) |= (flag))
#define CLEAR_FLAG(var, flag) ((var) &= ~(flag))
#define TOGGLE_FLAG(var, flag) ((var) ^= (flag))
#define HAS_FLAG(var, flag) (((var) & (flag)) != 0)

#define FILE_TO_WORK_ON "Demo.obj"
#define OUTPUT_LOG_STREAM stderr

#define REDDISH 255, 128, 128
#define GREENISH 128, 255, 128
#define BLUEISH 128, 128, 255
#define DARK_REDDISH 155, 28, 28
#define DARK_GREENISH 28, 155, 28
#define DARK_BLUEISH 28, 28, 155
#define BLACKISH 50, 50, 50
#define WHITISH 200, 200, 200

#define MAX_FPS 60
#define FRAME_DELAY (1000 / MAX_FPS)
#define INPUT_DELAY 75

typedef enum Enum_StatusCodes {
  // General states of a function that don't contribute in logging.
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

extern void Logger(Enum_StatusCodes *pStatus_codes,
                   const char *(*extra_logs_callback)(void), const char *logs,
                   FILE *output_stream);
