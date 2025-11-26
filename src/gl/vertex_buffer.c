#include "vertex_buffer.h"
#include "error.h"
#include <glad/glad.h>

void aa_vertex_buffer_create(aa_vertex_buffer* out)
{
  glCall(glGenBuffers(1, &out->id));
}

void aa_vertex_buffer_destroy(aa_vertex_buffer* out)
{
  glCall(glDeleteBuffers(1, &out->id));
}

void aa_vertex_buffer_bind(aa_vertex_buffer* out)
{
  glCall(glBindBuffer(GL_ARRAY_BUFFER, out->id));
}

void aa_vertex_buffer_update(aa_vertex_buffer* out, void* data, size_t data_size)
{
  glCall(glBindBuffer(GL_ARRAY_BUFFER, out->id));
  glCall(glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW));
}
