#ifndef __HG_AA_DARTBOARD
#define __HG_AA_DARTBOARD

#include <stdint.h>
#include "gl/vertex_array.h"
#include "gl/vertex_buffer.h"

// Struct to hold dartboard specific data
typedef struct
{
  aa_vertex_array vao;
  aa_vertex_buffer vbo;
  uint32_t vertex_count;
} DartboardScene;

// Initialize the dartboard geometry
int dartboard_init(DartboardScene* scene);

// Render the dartboard (binds VAO and draws)
void dartboard_render(DartboardScene* scene);

// Deletes dartboard specific data
void dartboard_cleanup(DartboardScene* scene);

#endif