#ifndef __HG_AA_GL_VERTEX_ARRAY
#define __HG_AA_GL_VERTEX_ARRAY

#include <glad/glad.h>
// Wrappers for OpenGl vertex arrays
typedef struct
{
  unsigned int id;
} aa_vertex_array;

void aa_vertex_array_create(aa_vertex_array* out);
void aa_vertex_array_delete(aa_vertex_array* out);
void aa_vertex_array_bind(aa_vertex_array* out);
void aa_vertex_array_position_attribute(aa_vertex_array* out);
void aa_vertex_array_position_color_attribute(aa_vertex_array* out);
void aa_vertex_array_position_uv_attribute(aa_vertex_array* out);
#endif // !__HG_AA_GL_VERTEX_BUFFER
