#ifndef __AA_HG_GL_ERROR
#define __AA_HG_GL_ERROR

#include <glad/glad.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

/// @brief Wraps an OpenGL function call to verify that no error was produced
#define glCall(x)                                       \
  do                                                    \
  {                                                     \
    x;                                                  \
    if (!aa_consume_log_errors(#x, __func__, __FILE__, __LINE__)) \
      abort();                                          \
  } while (0)

/// @brief Consumes the current OpenGL errors, and logs errors.
/// @param expression The expression
/// @param function The function name
/// @param file The file
/// @param line The line number
/// @return True if there is no errors
bool aa_consume_log_errors(
    const char* expression, const char* function, const char* file, uint32_t line);

#endif // !__AA_HG_GL_ERROR
