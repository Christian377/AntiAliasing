#include "frame_buffer.h"

void aa_texture_create(aa_texture* out, size_t width, size_t height)
{
  out->target = GL_TEXTURE_2D;
  glCall(glGenTextures(1, &out->id));
  glCall(glBindTexture(out->target, out->id));
  glCall(glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void aa_texture_msaa_create(
    aa_texture* out)
{
  out->target = GL_TEXTURE_2D_MULTISAMPLE;
  glCall(glGenTextures(1, &out->id));
}
//void aa_texture_msaa_create(
//    aa_texture* out, size_t width, size_t height, uint8_t samples)
//{
//  out->target = GL_TEXTURE_2D_MULTISAMPLE;
//  glCall(glGenTextures(1, &out->id));
//  glCall(glBindTexture(out->target, out->id));
//
//  glCall(glTexImage2DMultisample(
//      out->target, samples, GL_RGBA8, width, height, GL_TRUE));
//}

void aa_texture_bind(aa_texture* out)
{
  glCall(glBindTexture(out->target, out->id));
}

void aa_texture_delete(aa_texture* out)
{
  glCall(glDeleteTextures(1, &out->id));
}

void aa_texture_msaa_dimensions(
    aa_texture* out, size_t width, size_t height, uint8_t samples)
{
  glCall(glBindTexture(out->target, out->id));
  glCall(glTexImage2DMultisample(
      out->target, samples, GL_RGBA8, width, height, GL_TRUE));
}

void aa_texture_dimensions(aa_texture* out, size_t width, size_t height)
{
  glCall(glBindTexture(out->target, out->id));
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

void aa_frame_buffer_blit(
    aa_frame_buffer* out, aa_frame_buffer* multisample_fbo, size_t width,
    size_t height)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, multisample_fbo->id);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, out->id);

  glCall(glBlitFramebuffer(
      0, 0, width, height, 0, 0, width, height,
      GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

void aa_frame_buffer_color_texture(aa_frame_buffer* out, aa_texture* texture)
{
  glCall(glBindFramebuffer(GL_FRAMEBUFFER, out->id));
  glCall(glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->target, texture->id, 0));
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    printf("FBO error: 0x%x\n", status);
  }
}
