#include "program.h"
#include "error.h"
#include <glad/glad.h>

void aa_fragment_shader_create(aa_fragment_shader* out, const char* source)
{
  out->id = glCreateShader(GL_FRAGMENT_SHADER);
  glCall(glShaderSource(out->id, 1, &source, NULL));
}

void aa_vertex_shader_create(aa_vertex_shader* out, const char* source)
{
  out->id = glCreateShader(GL_VERTEX_SHADER);
  glCall(glShaderSource(out->id, 1, &source, NULL));
}

void aa_fragment_shader_source(aa_fragment_shader* out, const char* source)
{
  glCall(glShaderSource(out->id, 1, &source, NULL));
}

void aa_vertex_shader_source(aa_vertex_shader* out, const char* source)
{
  glCall(glShaderSource(out->id, 1, &source, NULL));
}

void aa_fragment_shader_compile(aa_fragment_shader* out)
{
  glCall(glCompileShader(out->id));
  int success;
  char infoLog[512];
  glCall(glGetShaderiv(out->id, GL_COMPILE_STATUS, &success));
  if (!success)
  {
    glCall(glGetShaderInfoLog(out->id, 512, NULL, infoLog));
    printf("ERROR: VERTEX SHADER COMPILATION FAILED\n%s", infoLog);
  }
}

void aa_vertex_shader_compile(aa_vertex_shader* out)
{
  glCall(glCompileShader(out->id));
  int success;
  char infoLog[512];
  glCall(glGetShaderiv(out->id, GL_COMPILE_STATUS, &success));
  if (!success)
  {
    glCall(glGetShaderInfoLog(out->id, 512, NULL, infoLog));
    printf("ERROR: FRAGMENT SHADER COMPILATION FAILED\n%s", infoLog);
  }
}

void aa_fragment_shader_delete(aa_fragment_shader* out)
{
  glCall(glDeleteShader(out->id));
}

void aa_vertex_shader_delete(aa_vertex_shader* out)
{
  glCall(glDeleteShader(out->id));
}

void aa_program_create(aa_program* out)
{
  out->id = glCreateProgram();
}

void aa_program_link(aa_program* out)
{
  glCall(glLinkProgram(out->id));
  int success;
  char infoLog[512];
  glCall(glGetProgramiv(out->id, GL_LINK_STATUS, &success));
  if (!success)
  {
    glCall(glGetProgramInfoLog(out->id, 512, NULL, infoLog));
    printf("ERROR: PROGRAM LINKING FAILED\n%s", infoLog);
  }
  glValidateProgram(out->id);
}

void aa_program_attach_shaders(
    aa_program* out, aa_vertex_shader* vertex_shader,
    aa_fragment_shader* fragment_shader)
{
  glCall(glAttachShader(out->id, vertex_shader->id));
  glCall(glAttachShader(out->id, fragment_shader->id));
}

void aa_program_use(aa_program* out)
{
  glCall(glUseProgram(out->id));
}
