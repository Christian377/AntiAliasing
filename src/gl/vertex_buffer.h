#ifndef __HG_AA_GL_VERTEX_BUFFER
#define __HG_AA_GL_VERTEX_BUFFER

#include <stddef.h>
#include <glad/glad.h>

// Wrappers for OpenGL vertex buffer objects
// Handles the allocation and data upload of vertex data (positions, colors, UVs)
// to the GPU memory

typedef struct
{
  unsigned int id;
} aa_vertex_buffer;

void aa_vertex_buffer_create(aa_vertex_buffer* out);
void aa_vertex_buffer_delete(aa_vertex_buffer* out);
void aa_vertex_buffer_bind(aa_vertex_buffer* out);
void aa_vertex_buffer_update(
    aa_vertex_buffer* out, const void* data, size_t data_size);

#endif // !__HG_AA_GL_VERTEX_BUFFER
