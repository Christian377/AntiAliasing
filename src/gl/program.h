#ifndef __HG_AA_GL_PROGRAM
#define __HG_AA_GL_PROGRAM

#include <glad/glad.h>
#include <stddef.h>
#include <stdio.h>

typedef struct
{
  unsigned int id;
} aa_fragment_shader;

typedef struct
{
  unsigned int id;
} aa_vertex_shader;

void aa_fragment_shader_create(aa_fragment_shader* out, const char* source);
void aa_vertex_shader_create(aa_vertex_shader* out, const char* source);
void aa_fragment_shader_source(aa_fragment_shader* out, const char* source);
void aa_vertex_shader_source(aa_vertex_shader* out, const char* source);
void aa_fragment_shader_compile(aa_fragment_shader* out);
void aa_vertex_shader_compile(aa_vertex_shader* out);
void aa_fragment_shader_delete(aa_fragment_shader* out);
void aa_vertex_shader_delete(aa_vertex_shader* out);

typedef struct
{
  unsigned int id;
} aa_program;

void aa_program_create(aa_program* out);
void aa_program_link(aa_program* out);
void aa_program_attach_shaders(
    aa_program* out, aa_vertex_shader* vertex_shader,
    aa_fragment_shader* fragment_shader);
void aa_program_use(aa_program* out);
void aa_program_delete(aa_program* out);

#endif // !__HG_AA_GL_PROGRAM
