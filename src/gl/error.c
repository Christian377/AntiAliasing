#include "error.h"

bool aa_consume_log_errors(
    const char* expression, const char* function, const char* file, uint32_t line)
{
  bool ret = true;
  GLenum error;
  while (error = glGetError())
  {
    const char* toPrint = "<ERROR>";
    switch (error)
    {
    case GL_INVALID_ENUM:
      toPrint = "INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      toPrint = "INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      toPrint = "INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      toPrint = "STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      toPrint = "STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      toPrint = "OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      toPrint = "INVALID_FRAMEBUFFER_OPERATION";
    }
    printf(
        "OpenGL: File `%s` at line %" PRIu32 ":\nFunction: %s: %s -> %s\n", file,
        line, function, expression, toPrint);
    ret = false;
  }
  return ret;
}
