#include "../include/common.h"

void Logger(Enum_StatusCodes *pStatus_codes,
            const char *(*extra_logs_callback)(void), const char *logs,
            FILE *output_stream) {
  const char *callback_logs = "";
  if (extra_logs_callback) {
    callback_logs = extra_logs_callback();
    if (!callback_logs) {
      callback_logs = "No function callbacks provided.\n";
    }
  }

  if (!logs) {
    logs = "No logs provided.\n";
  }

  if (!output_stream) {
    output_stream = stderr;
  }

  fprintf(output_stream, "\nLOGS:\n");
  fprintf(output_stream, "  FUNCTION CALLBACK'S LOGS:\n    %s", callback_logs);
  fprintf(output_stream, "  LOGS:\n    %s", logs);
  fprintf(output_stream, "  STATUS CODE'S LOGS:\n    ");

  if (*pStatus_codes) {
    if (HAS_FLAG(*pStatus_codes, HIGH_SEVERITY_ERROR)) {
      fprintf(output_stream, "    [ERROR]: ");
    } else if (HAS_FLAG(*pStatus_codes, LOW_SEVERITY_ERROR)) {
      fprintf(output_stream, "    [WARNING]: ");
    }
    /*
    In the current project scope, encountering multiple errors per Logger call
    is highly unlikely. Therefore, we are not implementing multi-error handling
    for now. This can be reconsidered if requirements change.
    */
    if (HAS_FLAG(*pStatus_codes, MEM_ALLOC_FAILURE)) {
      fprintf(output_stream,
              "Unable to allocate memory, malloc/calloc/realloc failure.\n");
    } else if (HAS_FLAG(*pStatus_codes, INVALID_FUNCTION_INPUT)) {
      fprintf(output_stream, "Invalid/NULL inputs passed to a functions.\n");
    } else if (HAS_FLAG(*pStatus_codes, INVALID_FILE_PATH)) {
      fprintf(output_stream, "Invalid file path provided.\n");
    } else if (HAS_FLAG(*pStatus_codes, DEPENDENCY_FAILURE)) {
      fprintf(output_stream,
              "Some internal dependency failed to give desired results.\n");
    } else if (HAS_FLAG(*pStatus_codes, UNEXPECTED_COMPUTED_RESULTS)) {
      fprintf(
          output_stream,
          "Produced unexpected results. Most likely invalid data provided.\n");
    } else {
      fprintf(output_stream, "No status codes provided.\n");
    }
  }
}