#ifndef __HG_AA_GL_FRAME_BUFFER
#define __HG_AA_GL_FRAME_BUFFER

#include <glad/glad.h>
#include <stdint.h>
#include "error.h"

typedef struct
{
  unsigned int id;
  GLenum target;
} aa_texture;

void aa_texture_create(aa_texture* out, size_t width, size_t height);
void aa_texture_msaa_create(aa_texture* out);
void aa_texture_from_data(
    aa_texture* out, const unsigned char* data, size_t width, size_t height);
void aa_smaa_search_texture(
    aa_texture* out, const unsigned char* data, int width, int height);
void aa_smaa_area_texture(
    aa_texture* out, const unsigned char* data, int width, int height);
void aa_texture_bind(aa_texture* out);
void aa_texture_delete(aa_texture* out);
void aa_texture_dimensions(aa_texture* out, size_t width, size_t height);
void aa_texture_msaa_dimensions(
    aa_texture* out, size_t width, size_t height, uint8_t samples);

typedef struct
{
  unsigned int id;
} aa_frame_buffer;

void aa_frame_buffer_create(aa_frame_buffer* out);
void aa_frame_buffer_bind(aa_frame_buffer* out);
void aa_frame_buffer_delete(aa_frame_buffer* out);
void aa_frame_buffer_color_texture(aa_frame_buffer* out, aa_texture* texture);
void aa_frame_buffer_blit(
    aa_frame_buffer* out, aa_frame_buffer* multisample_fbo, size_t width,
    size_t height);

#endif // !__HG_AA_GL_FRAME_BUFFER
