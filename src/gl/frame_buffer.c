#include "frame_buffer.h"

void aa_texture_create(aa_texture* out, size_t width, size_t height)
{
  glCall(glGenTextures(1, &out->id));
  glCall(glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void aa_texture_bind(aa_texture* out)
{
  glCall(glBindTexture(GL_TEXTURE_2D, out->id));
}

void aa_texture_delete(aa_texture* out)
{
  glCall(glDeleteTextures(1, &out->id));
}

void aa_texture_dimensions(aa_texture* out, size_t width, size_t height)
{
  glCall(glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
}

void aa_frame_buffer_create(aa_frame_buffer* out)
{
  glCall(glGenFramebuffers(1, &out->id));
}

void aa_frame_buffer_bind(aa_frame_buffer* out)
{
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, out->id));
}

void aa_frame_buffer_delete(aa_frame_buffer* out)
{
  glCall(glDeleteFramebuffers(1, &out->id));
}

void aa_bind_color_texture(aa_frame_buffer* out, aa_texture* texture)
{
  glCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0));
}

