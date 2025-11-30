#ifndef __HG_AA_GL_VERTEX_BUFFER
#define __HG_AA_GL_VERTEX_BUFFER

#include <stddef.h>
#include <glad/glad.h>

typedef struct
{
  unsigned int id;
} aa_vertex_buffer;

void aa_vertex_buffer_create(aa_vertex_buffer* out);
void aa_vertex_buffer_destroy(aa_vertex_buffer* out);
void aa_vertex_buffer_bind(aa_vertex_buffer* out);
void aa_vertex_buffer_update(aa_vertex_buffer* out, const void* data, size_t data_size);

#endif // !__HG_AA_GL_VERTEX_BUFFER
