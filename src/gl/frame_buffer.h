#ifndef __HG_AA_GL_FRAME_BUFFER
#define __HG_AA_GL_FRAME_BUFFER

#include <glad/glad.h>
#include "error.h"

typedef struct
{
  unsigned int id;
}aa_texture;

void aa_texture_create(aa_texture* out, size_t width, size_t height);
void aa_texture_bind(aa_texture* out);
void aa_texture_delete(aa_texture* out);
void aa_texture_dimensions(aa_texture* out, size_t width, size_t height);

typedef struct
{
  unsigned int id;
}aa_frame_buffer;

void aa_frame_buffer_create(aa_frame_buffer* out);
void aa_frame_buffer_bind(aa_frame_buffer* out);
void aa_frame_buffer_delete(aa_frame_buffer* out);
void aa_bind_color_texture(aa_frame_buffer* out, aa_texture texture);



#endif // !__HG_AA_GL_FRAME_BUFFER
