#include "vertex_array.h"
#include "error.h"
#include <glad/glad.h>

void aa_vertex_array_create(aa_vertex_array* out)
{
  glCall(glGenVertexArrays(1, &out->id));
}

void aa_vertex_array_delete(aa_vertex_array* out)
{
  glCall(glDeleteVertexArrays(1, &out->id));
}

void aa_vertex_array_bind(aa_vertex_array* out)
{
  glCall(glBindVertexArray(out->id));
}

void aa_vertex_array_position_attribute(aa_vertex_array* out)
{
  glCall(glBindVertexArray(out->id));
  glCall(
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
  glCall(glEnableVertexAttribArray(0));
}

// TODO: fix me
void aa_vertex_array_position_color_attribute(aa_vertex_array* out)
{
  glCall(glBindVertexArray(out->id));
  glCall(
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
  glCall(glEnableVertexAttribArray(0));
  glCall(
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
  glCall(glEnableVertexAttribArray(1));
}

void aa_vertex_array_position_uv_attribute(aa_vertex_array* out)
{
  glCall(glBindVertexArray(out->id));
  glCall(
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
  glCall(glEnableVertexAttribArray(0));
  glCall(
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
  glCall(glEnableVertexAttribArray(1));
}
