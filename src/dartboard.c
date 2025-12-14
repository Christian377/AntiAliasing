#include "dartboard.h"
#include <math.h>
#include <stdlib.h>
#include <glad/glad.h>

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

// In dartboard.c

int dartboard_init(DartboardScene* scene)
{
  // 180 slices total, we only draw half of them (90 spokes)
  int total_slices    = 180;
  int spokes          = total_slices / 2;
  scene->vertex_count = spokes * 3;

  // Position only (3 floats per vertex)
  size_t buffer_size = scene->vertex_count * 3 * sizeof(float);
  float* vertices    = (float*)malloc(buffer_size);

  float radius = 1.5f;

  int idx = 0;
  for (int i = 0; i < total_slices; i += 2)
  {
    float theta1 = ((float)i / total_slices) * 2.0f * M_PI;
    float theta2 = ((float)(i + 1) / total_slices) * 2.0f * M_PI;

    // Center Vertex
    vertices[idx++] = 0.0f;
    vertices[idx++] = 0.0f;
    vertices[idx++] = 0.0f;

    // Rim Vertex 1
    vertices[idx++] = radius * cosf(theta1);
    vertices[idx++] = radius * sinf(theta1);
    vertices[idx++] = 0.0f;

    // Rim Vertex 2
    vertices[idx++] = radius * cosf(theta2);
    vertices[idx++] = radius * sinf(theta2);
    vertices[idx++] = 0.0f;
  }


  // Create and fill the VBO
  aa_vertex_buffer_create(&scene->vbo);
  aa_vertex_buffer_update(&scene->vbo, vertices, buffer_size);

  // Create the VAO
  aa_vertex_array_create(&scene->vao);

  // 3Bind the VAO so we can configure it
  aa_vertex_array_bind(&scene->vao);

  // Bind the VBO while the VAO is bound (Crucial Step!)
  aa_vertex_buffer_bind(&scene->vbo);

  // layout location = 0
  aa_vertex_array_position_attribute(&scene->vao);

  free(vertices);
  return 0;
}

void dartboard_render(DartboardScene* scene)
{
  aa_vertex_array_bind(&scene->vao);
  glDrawArrays(GL_TRIANGLES, 0, scene->vertex_count);
}

void dartboard_cleanup(DartboardScene* scene)
{
  aa_vertex_buffer_delete(&scene->vbo);
  aa_vertex_array_delete(&scene->vao);
}