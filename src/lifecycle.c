#include "lifecycle.h"

int on_init(AppState* state)
{
  // triangle vertices position
  static const float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                   0.0f,  0.0f,  0.5f, 0.0f};
  // fullscreenquad positions and uv coordinates, used for fxaa
  static const float fullscreen_vertices[] = {
      -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, -1.0f, 0.0f,
      1.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,

      -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f,
      1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

  // Basic shaders
  char* VERTEX_DEFAULT   = aa_load_file("resources/shaders/vertex_default.glsl");
  char* FRAGMENT_DEFAULT = aa_load_file("resources/shaders/fragment_default.glsl");

  char* VERTEX_FULLSCREEN_QUAD =
      aa_load_file("resources/shaders/vertex_fullscreen_quad.glsl");
  char* FRAGMENT_FXAA = aa_load_file("resources/shaders/fragment_fxaa.glsl");
  char* FRAGMENT_FXAA_ITER =
      aa_load_file("resources/shaders/fragment_fxaa_iterative.glsl");

  // SMAA shaders common part across settings
  char* VERTEX_EDGE_SMAA_BODY =
      aa_load_file("resources/shaders/vertex_edge_smaa.glsl");
  char* VERTEX_BLEND_SMAA_BODY =
      aa_load_file("resources/shaders/vertex_blend_smaa.glsl");
  char* VERTEX_NEIGHBORHOOD_SMAA_BODY =
      aa_load_file("resources/shaders/vertex_neighborhood_smaa.glsl");
  char* FRAGMENT_EDGE_SMAA_BODY =
      aa_load_file("resources/shaders/fragment_edge_smaa.glsl");
  char* FRAGMENT_BLEND_SMAA_BODY =
      aa_load_file("resources/shaders/fragment_blend_smaa.glsl");
  char* FRAGMENT_NEIGHBORHOOD_SMAA_BODY =
      aa_load_file("resources/shaders/fragment_neighborhood_smaa.glsl");

  // Assembling SMAA shaders
  char* SMAA_LIB = aa_load_file("resources/shaders/SMAA.hlsl");
  if (!SMAA_LIB)
  {
    printf("Error loading SMAA.hlsl\n");
    return -1;
  }
  // LOW
  aa_smaa_pipeline_init(
      &state->smaa_low, "#define SMAA_PRESET_LOW 1\n", SMAA_LIB,
      VERTEX_EDGE_SMAA_BODY, FRAGMENT_EDGE_SMAA_BODY, VERTEX_BLEND_SMAA_BODY,
      FRAGMENT_BLEND_SMAA_BODY, VERTEX_NEIGHBORHOOD_SMAA_BODY,
      FRAGMENT_NEIGHBORHOOD_SMAA_BODY);
  // MEDIUM
  aa_smaa_pipeline_init(
      &state->smaa_medium, "#define SMAA_PRESET_MEDIUM 1\n", SMAA_LIB,
      VERTEX_EDGE_SMAA_BODY, FRAGMENT_EDGE_SMAA_BODY, VERTEX_BLEND_SMAA_BODY,
      FRAGMENT_BLEND_SMAA_BODY, VERTEX_NEIGHBORHOOD_SMAA_BODY,
      FRAGMENT_NEIGHBORHOOD_SMAA_BODY);
  
  // HIGH
  aa_smaa_pipeline_init(
      &state->smaa_high, "#define SMAA_PRESET_HIGH 1\n", SMAA_LIB,
      VERTEX_EDGE_SMAA_BODY, FRAGMENT_EDGE_SMAA_BODY, VERTEX_BLEND_SMAA_BODY,
      FRAGMENT_BLEND_SMAA_BODY, VERTEX_NEIGHBORHOOD_SMAA_BODY,
      FRAGMENT_NEIGHBORHOOD_SMAA_BODY);
  // ULTRA
  aa_smaa_pipeline_init(
      &state->smaa_ultra, "#define SMAA_PRESET_ULTRA 1\n", SMAA_LIB,
      VERTEX_EDGE_SMAA_BODY, FRAGMENT_EDGE_SMAA_BODY, VERTEX_BLEND_SMAA_BODY,
      FRAGMENT_BLEND_SMAA_BODY, VERTEX_NEIGHBORHOOD_SMAA_BODY,
      FRAGMENT_NEIGHBORHOOD_SMAA_BODY);

  // Sampling logic
  state->samples_total   = AA_SAMPLE_COUNT;
  state->samples_current = 0;
  size_t samples_bytes   = sizeof(uint32_t) * state->samples_total;
  state->samples         = malloc(samples_bytes);
  state->is_recording    = false;
  if (state->samples == NULL)
    return -1;
  memset(state->samples, 0, samples_bytes);
  state->current_algorithm_file_name = "aa_NONE.txt";

  // Check if any file failed to load
  if (VERTEX_DEFAULT == NULL || FRAGMENT_DEFAULT == NULL
      || VERTEX_FULLSCREEN_QUAD == NULL || FRAGMENT_FXAA == NULL
      || FRAGMENT_FXAA_ITER == NULL || SMAA_LIB == NULL
      || VERTEX_EDGE_SMAA_BODY == NULL || VERTEX_BLEND_SMAA_BODY == NULL
      || VERTEX_NEIGHBORHOOD_SMAA_BODY == NULL || FRAGMENT_EDGE_SMAA_BODY == NULL
      || FRAGMENT_BLEND_SMAA_BODY == NULL || FRAGMENT_NEIGHBORHOOD_SMAA_BODY == NULL)
  {
    free(VERTEX_DEFAULT);
    free(FRAGMENT_DEFAULT);
    free(VERTEX_FULLSCREEN_QUAD);
    free(FRAGMENT_FXAA);
    free(FRAGMENT_FXAA_ITER);
    free(SMAA_LIB);
    free(VERTEX_EDGE_SMAA_BODY);
    free(VERTEX_BLEND_SMAA_BODY);
    free(VERTEX_NEIGHBORHOOD_SMAA_BODY);
    free(FRAGMENT_EDGE_SMAA_BODY);
    free(FRAGMENT_BLEND_SMAA_BODY);
    free(FRAGMENT_NEIGHBORHOOD_SMAA_BODY);
    printf("Error: One or more shader files failed to load.\n");
    return -1;
  }
  // Create time query
  aa_time_query_create(&state->query);

  // create programs
  aa_program_create(&state->program);
  aa_program_create(&state->fxaa_program);
  aa_program_create(&state->fxaa_iterative_program);

  // create shaders
  aa_vertex_shader_create(&state->default_vertex_shader, VERTEX_DEFAULT);
  aa_fragment_shader_create(&state->default_fragment_shader, FRAGMENT_DEFAULT);
  aa_vertex_shader_create(
      &state->fullscreen_quad_vertex_shader, VERTEX_FULLSCREEN_QUAD);
  aa_fragment_shader_create(&state->fxaa_fragment_shader, FRAGMENT_FXAA);
  aa_fragment_shader_create(
      &state->fxaa_iterative_fragment_shader, FRAGMENT_FXAA_ITER);

  // compile shaders
  aa_vertex_shader_compile(&state->default_vertex_shader);
  aa_fragment_shader_compile(&state->default_fragment_shader);
  aa_vertex_shader_compile(&state->fullscreen_quad_vertex_shader);
  aa_fragment_shader_compile(&state->fxaa_fragment_shader);
  aa_fragment_shader_compile(&state->fxaa_iterative_fragment_shader);

  // attach shaders and link programs
  aa_program_attach_shaders(
      &state->fxaa_program, &state->fullscreen_quad_vertex_shader,
      &state->fxaa_fragment_shader);
  aa_program_link(&state->fxaa_program);

  aa_program_attach_shaders(
      &state->fxaa_iterative_program, &state->fullscreen_quad_vertex_shader,
      &state->fxaa_iterative_fragment_shader);
  aa_program_link(&state->fxaa_iterative_program);

  aa_program_attach_shaders(
      &state->program, &state->default_vertex_shader,
      &state->default_fragment_shader);
  aa_program_link(&state->program);

  // triangle vao and vbo setup
  aa_vertex_buffer_create(&state->vbo);
  aa_vertex_buffer_update(&state->vbo, vertices, sizeof(vertices));

  aa_vertex_array_create(&state->vao);
  aa_vertex_array_position_attribute(&state->vao);

  // fullscreen quad vao and vbo setup
  aa_vertex_buffer_create(&state->fullscreen_vbo);
  aa_vertex_buffer_update(
      &state->fullscreen_vbo, fullscreen_vertices, sizeof(fullscreen_vertices));
  aa_vertex_array_create(&state->fullscreen_vao);
  aa_vertex_array_position_uv_attribute(&state->fullscreen_vao);

  // MSAA multisampling fbo and texture initialization
  aa_frame_buffer_create(&state->msaa_fbo_x4);
  aa_frame_buffer_create(&state->msaa_fbo_x8);
  aa_frame_buffer_create(&state->msaa_fbo_x16);

  aa_texture_msaa_create(&state->msaa_color_texture_x4);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x4, state->window_width, state->window_height, 4);

  aa_texture_msaa_create(&state->msaa_color_texture_x8);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x8, state->window_width, state->window_height, 8);

  aa_texture_msaa_create(&state->msaa_color_texture_x16);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x16, state->window_width, state->window_height, 16);
  aa_frame_buffer_color_texture(&state->msaa_fbo_x4, &state->msaa_color_texture_x4);
  aa_frame_buffer_color_texture(&state->msaa_fbo_x8, &state->msaa_color_texture_x8);
  aa_frame_buffer_color_texture(
      &state->msaa_fbo_x16, &state->msaa_color_texture_x16);
  aa_frame_buffer_bind(&state->default_fbo);

  // FXAA fbo and screen texture initialization
  aa_frame_buffer_create(&state->fxaa_fbo);
  aa_texture_create(
      &state->fxaa_color_texture, state->window_width, state->window_height);
  aa_frame_buffer_color_texture(&state->fxaa_fbo, &state->fxaa_color_texture);

  // FXAA fbo and required textures initialization
  aa_frame_buffer_create(&state->smaa_fbo);
  aa_frame_buffer_create(&state->smaa_edge_fbo);
  aa_frame_buffer_create(&state->smaa_blend_fbo);
  aa_smaa_area_texture(&state->smaa_area_texture, areaTexBytes, 160, 560);
  aa_smaa_search_texture(&state->smaa_search_texture, searchTexBytes, 60, 33);
  aa_texture_create(
      &state->smaa_color_texture, state->window_width, state->window_height);
  aa_texture_create(
      &state->smaa_edge_texture, state->window_width, state->window_height);
  aa_texture_create(
      &state->smaa_blend_texture, state->window_width, state->window_height);
  aa_frame_buffer_color_texture(&state->smaa_fbo, &state->smaa_color_texture);
  aa_frame_buffer_color_texture(&state->smaa_edge_fbo, &state->smaa_edge_texture);
  aa_frame_buffer_color_texture(&state->smaa_blend_fbo, &state->smaa_blend_texture);

  // Free uncompiled shaders memory
  free(VERTEX_DEFAULT);
  free(FRAGMENT_DEFAULT);
  free(VERTEX_FULLSCREEN_QUAD);
  free(FRAGMENT_FXAA);
  free(FRAGMENT_FXAA_ITER);

  if (dartboard_init(&state->dartboard) != 0)
  {
    printf("Error initializing dartboard\n");
    return -1;
  }

  return 0;
}

void on_end(AppState* state)
{
  // Delete Programs
  aa_program_delete(&state->program);
  aa_program_delete(&state->fxaa_program);
  aa_program_delete(&state->fxaa_iterative_program);

  // Delete Shaders
  aa_vertex_shader_delete(&state->default_vertex_shader);
  aa_fragment_shader_delete(&state->default_fragment_shader);
  aa_vertex_shader_delete(&state->fullscreen_quad_vertex_shader);
  aa_fragment_shader_delete(&state->fxaa_fragment_shader);
  aa_fragment_shader_delete(&state->fxaa_iterative_fragment_shader);

  // Delete Buffers and vaos
  aa_vertex_buffer_delete(&state->vbo);
  aa_vertex_array_delete(&state->vao);
  aa_vertex_buffer_delete(&state->fullscreen_vbo);
  aa_vertex_array_delete(&state->fullscreen_vao);

  // Delete Framebuffers
  aa_frame_buffer_delete(&state->msaa_fbo_x4);
  aa_frame_buffer_delete(&state->msaa_fbo_x8);
  aa_frame_buffer_delete(&state->msaa_fbo_x16);
  aa_frame_buffer_delete(&state->fxaa_fbo);
  aa_frame_buffer_delete(&state->smaa_fbo);
  aa_frame_buffer_delete(&state->smaa_edge_fbo);
  aa_frame_buffer_delete(&state->smaa_blend_fbo);

  // Delete Textures
  aa_texture_delete(&state->msaa_color_texture_x4);
  aa_texture_delete(&state->msaa_color_texture_x8);
  aa_texture_delete(&state->msaa_color_texture_x16);
  aa_texture_delete(&state->fxaa_color_texture);
  aa_texture_delete(&state->smaa_color_texture);
  aa_texture_delete(&state->smaa_area_texture);
  aa_texture_delete(&state->smaa_search_texture);
  aa_texture_delete(&state->smaa_edge_texture);
  aa_texture_delete(&state->smaa_blend_texture);

  // Delete SMAA Pipelines
  aa_smaa_pipeline_delete(&state->smaa_low);
  aa_smaa_pipeline_delete(&state->smaa_medium);
  aa_smaa_pipeline_delete(&state->smaa_high);
  aa_smaa_pipeline_delete(&state->smaa_ultra);

  // Delete Query
  aa_time_query_delete(&state->query);
  free(state->samples);

  // Delete Dartboard Scene Specific Data
  dartboard_cleanup(&state->dartboard);
}

void on_resize(AppState* state)
{
  // Resize textures and rebind to corresponding fbos
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x4, state->window_width, state->window_height, 4);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x8, state->window_width, state->window_height, 8);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture_x16, state->window_width, state->window_height, 16);
  aa_frame_buffer_color_texture(&state->msaa_fbo_x4, &state->msaa_color_texture_x4);
  aa_frame_buffer_color_texture(&state->msaa_fbo_x8, &state->msaa_color_texture_x8);
  aa_frame_buffer_color_texture(
      &state->msaa_fbo_x16, &state->msaa_color_texture_x16);
  aa_texture_dimensions(
      &state->fxaa_color_texture, state->window_width, state->window_height);
  aa_texture_dimensions(
      &state->smaa_color_texture, state->window_width, state->window_height);
  aa_texture_dimensions(
      &state->smaa_edge_texture, state->window_width, state->window_height);
  aa_texture_dimensions(
      &state->smaa_blend_texture, state->window_width, state->window_height);
  aa_frame_buffer_color_texture(&state->fxaa_fbo, &state->fxaa_color_texture);
  aa_frame_buffer_color_texture(&state->smaa_fbo, &state->smaa_color_texture);
  aa_frame_buffer_color_texture(&state->smaa_edge_fbo, &state->smaa_edge_texture);
  aa_frame_buffer_color_texture(&state->smaa_blend_fbo, &state->smaa_blend_texture);
  aa_frame_buffer_bind(&state->default_fbo);
}