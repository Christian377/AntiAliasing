#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include <cimgui.h>
#include <cimgui_impl.h>

#include "gl/program.h"
#include "gl/shaders.h"
#include "gl/vertex_array.h"
#include "gl/vertex_buffer.h"
#include "gl/frame_buffer.h"
#include "gl/query.h"
#include "smaa/AreaTex.h"
#include "smaa/SearchTex.h"
#include "smaa_helper.h"
#include "dartboard.h"

#ifdef _WIN32
// on windows define the following symbols so that the high performance
// GPU is used (not the integrated one).
__declspec(dllexport) uint32_t NvOptimusEnablement             = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

/// @brief The number of samples to try to capture
uint32_t AA_SAMPLE_COUNT = 100;

// Anti Aliasing Options
typedef enum
{
  AA_NONE,
  AA_MSAAx4,
  AA_MSAAx8,
  AA_MSAAx16,
  AA_FXAA,
  AA_FXAA_ITERATIVE,
  AA_SMAA_LOW,
  AA_SMAA_MEDIUM,
  AA_SMAA_HIGH,
  AA_SMAA_ULTRA
} aa_algorithm;

// The scene to draw
typedef enum
{
  SCENE_TRIANGLE,
  SCENE_DARTBOARD
} SceneType;

/// @brief Application state, across frames
typedef struct
{
  /// @brief The window (never null)
  GLFWwindow* const window;
  /// @brief The ImGUI context (never null)
  ImGuiContext* const imgui_context;
  /// @brief The ImGUI IO of `imgui_context` (never null)
  ImGuiIO* const imgui_io;
  /// @brief The window's width
  int window_width;
  /// @brief The window's height
  int window_height;
  /// @brief The frame count
  uint64_t frame_count;
  /// @brief The delta time between frames (in seconds)
  double delta_time;
  /// @brief The time passed from first frame until present
  double elapsed_time;
  /// @brief The currently used anti aliasing algorithm
  aa_algorithm anti_aliasing;
  /// @brief The time query responsible for measuring algorithms performance
  aa_time_query query;
  /// @brief The vertex array object for position only vertices
  aa_vertex_array vao;
  /// @brief The position only vertex buffer object
  aa_vertex_buffer vbo;
  /// @brief The veryex array object for position and uv attributes vertices
  aa_vertex_array fullscreen_vao;
  /// @brief The position and uv attributes vertex buffer object
  aa_vertex_buffer fullscreen_vbo;
  /// @brief Program used to draw the scene without any specific additional effect
  aa_program program;
  /// @brief Program used to apply FXAA
  aa_program fxaa_program;
  /// @brief Program used to apply FXAA
  aa_program fxaa_iterative_program;
  /// @brief Vertex shader which takes as input position only vertices
  aa_fragment_shader default_fragment_shader;
  /// @brief Fragment shader which simply draws vertex shader outputs without any additional effect
  aa_vertex_shader default_vertex_shader;
  /// @brief Fragment shader containing FXAA post processing algorithm
  aa_fragment_shader fxaa_fragment_shader;
  /// @brief Fragment shader containing FXAA post processing algorithm
  aa_fragment_shader fxaa_iterative_fragment_shader;
  /// @brief Vertex shader used to render a texture on the screen
  aa_vertex_shader fullscreen_quad_vertex_shader;
  // SMAA Pipelines (Programs + Shaders bundled)
  aa_smaa_pipeline smaa_low;
  aa_smaa_pipeline smaa_medium;
  aa_smaa_pipeline smaa_high;
  aa_smaa_pipeline smaa_ultra;
  // Default fbo, with id 0
  aa_frame_buffer default_fbo;
  // MSAA multisampling fbo and textures
  aa_frame_buffer msaa_fbo_x4;
  aa_frame_buffer msaa_fbo_x8;
  aa_frame_buffer msaa_fbo_x16;
  aa_texture msaa_color_texture_x4;
  aa_texture msaa_color_texture_x8;
  aa_texture msaa_color_texture_x16;
  // FXAA fbo and screen texture
  aa_frame_buffer fxaa_fbo;
  aa_texture fxaa_color_texture;
  // FXAA fbo and required textures
  aa_frame_buffer smaa_fbo;
  aa_frame_buffer smaa_edge_fbo;
  aa_frame_buffer smaa_blend_fbo;
  aa_texture smaa_color_texture;
  aa_texture smaa_area_texture;
  aa_texture smaa_search_texture;
  aa_texture smaa_edge_texture;
  aa_texture smaa_blend_texture;
  // File in which we write the time values
  const char* current_algorithm_file_name;
  // samples buffer
  bool is_recording;
  uint64_t samples_total;
  uint64_t samples_current;
  uint32_t* samples;
  // Automation flags
  bool automation_mode;
  int warmup_frames;
  /// @brief The scene to be drawn
  SceneType current_scene;
  /// @brief The dartboard scene data
  DartboardScene dartboard;
} AppState;

// Function handling draw calls, based on current scene to render
static void render_current_scene(AppState* state)
{
  // Both scenes use the default program
  aa_program_use(&state->program);

  if (state->current_scene == SCENE_TRIANGLE)
  {
    aa_vertex_array_bind(&state->vao);
    aa_vertex_buffer_bind(&state->vbo);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
  else if (state->current_scene == SCENE_DARTBOARD)
  {
    dartboard_render(&state->dartboard);
  }
}

static void run_automation_logic(AppState* state)
{
  // Only run if automation is enabled
  if (!state->automation_mode)
    return;

  // Wait 20 frames to let GPU stabilize
  if (state->warmup_frames > 0)
  {
    state->warmup_frames--;
    if (state->warmup_frames == 0)
    {
      // Start recording.
      state->is_recording    = true;
      state->samples_current = 0; // Reset counter
    }
    return;
  }

  // Check if Recording is Done
  if (state->is_recording && state->samples_current >= state->samples_total)
  {
    // Stop Recording
    state->is_recording = false;

    // Save Samples
    if (state->current_algorithm_file_name)
    {
      FILE* file = fopen(state->current_algorithm_file_name, "w");
      if (file)
      {
        for (size_t i = 0; i < state->samples_current; i++)
          fprintf(file, "%" PRIu32 ",", state->samples[i]);
        fclose(file);
        printf("Saved: %s\n", state->current_algorithm_file_name);
      }
    }

    // Move to Next Algorithm
    state->anti_aliasing++;

    // Reset for the new algorithm
    state->warmup_frames = 100;

    // Check if we went past the last algorithm
    if (state->anti_aliasing > AA_SMAA_ULTRA)
    {
      if (state->current_scene == SCENE_TRIANGLE)
      {
        printf("Triangle finished. Switching to Dartboard.\n");
        state->current_scene = SCENE_DARTBOARD;
        state->anti_aliasing = AA_NONE; 
        state->warmup_frames = 100;     
      }
      else
      {
        printf("All algorithms and scenes finished. Closing.\n");
        glfwSetWindowShouldClose(state->window, true);
      }
    }
  }
}

// Function called once per frame
static void on_frame(AppState* state)
{
  run_automation_logic(state);
  if (glfwWindowShouldClose(state->window))
    return;
  // Ensure ImGui render state doesn't interfere with full-screen rendering
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glViewport(0, 0, state->window_width, state->window_height);
  glClearColor(
      fabsf(sinf((float)state->elapsed_time * 1.4f)),
      fabsf(sinf((float)state->elapsed_time * 1.1f)),
      fabsf(sinf((float)state->elapsed_time * 0.8f)), 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!state->automation_mode)
  {
    // Setting up the control window (only in manual mode)
    if (igBegin("Control", NULL, 0))
    {
      // Algorithm selection menu
      igTextColored((ImVec4){1.0f, 0.9f, 0.0f, 1.0f}, "Anti-Aliasing Algorithm:");
      if (igButton("No AA", (ImVec2){0, 0}))
        state->anti_aliasing = AA_NONE;
      igSameLine(0.0f, 5.0f);
      if (igButton("MSAA_x4", (ImVec2){0, 0}))
        state->anti_aliasing = AA_MSAAx4;
      igSameLine(0.0f, 5.0f);
      if (igButton("MSAA_x8", (ImVec2){0, 0}))
        state->anti_aliasing = AA_MSAAx8;
      igSameLine(0.0f, 5.0f);
      if (igButton("MSAA_x16", (ImVec2){0, 0}))
        state->anti_aliasing = AA_MSAAx16;
      igSameLine(0.0f, 5.0f);
      if (igButton("FXAA", (ImVec2){0, 0}))
        state->anti_aliasing = AA_FXAA;
      // New line
      if (igButton("FXAA_iter", (ImVec2){0, 0}))
        state->anti_aliasing = AA_FXAA_ITERATIVE;
      igSameLine(0.0f, 5.0f);
      if (igButton("SMAA_LOW", (ImVec2){0, 0}))
        state->anti_aliasing = AA_SMAA_LOW;
      igSameLine(0.0f, 5.0f);
      if (igButton("SMAA_MEDIUM", (ImVec2){0, 0}))
        state->anti_aliasing = AA_SMAA_MEDIUM;
      igSameLine(0.0f, 5.0f);
      if (igButton("SMAA_HIGH", (ImVec2){0, 0}))
        state->anti_aliasing = AA_SMAA_HIGH;
      igSameLine(0.0f, 5.0f);
      if (igButton("SMAA_ULTRA", (ImVec2){0, 0}))
        state->anti_aliasing = AA_SMAA_ULTRA;
      //Scene Menu
      igSeparator();
      igTextColored((ImVec4){1.0f, 0.9f, 0.0f, 1.0f}, "Scene Selection:");
      if (igButton("Triangle", (ImVec2){0, 0}))
        state->current_scene = SCENE_TRIANGLE;
      igSameLine(0.0f, 5.0f);
      if (igButton("Dartboard", (ImVec2){0, 0}))
        state->current_scene = SCENE_DARTBOARD;
      igSeparator();
      // Tracing Menu
      igTextColored((ImVec4){1.0f, 0.9f, 0.0f, 1.0f}, "Tracing:");
      igBeginDisabled(state->is_recording);
      if (igInputInt("Number of Samples", &AA_SAMPLE_COUNT, 10, 100, 0))
      {
        if (AA_SAMPLE_COUNT > 10000)
          AA_SAMPLE_COUNT = 10000;
        if (AA_SAMPLE_COUNT < 10)
          AA_SAMPLE_COUNT = 10;
        state->samples_total = AA_SAMPLE_COUNT;
        state->samples = realloc(state->samples, AA_SAMPLE_COUNT * sizeof(uint32_t));
        state->samples_current = 0;
      }
      igEndDisabled();

      if (igButton("Record Samples", (ImVec2){0, 0}))
      {
        state->samples_current = 0;
        state->is_recording    = true;
      }
      igSameLine(0.0f, 5.0f);
      igBeginDisabled(state->samples_current != state->samples_total);
      if (igButton("Save Samples", (ImVec2){0, 0}))
      {
        FILE* file = fopen(state->current_algorithm_file_name, "w");
        if (file == NULL)
        {
          printf(
              "Error: Could not create file `%s`!",
              state->current_algorithm_file_name);
        }
        else
        {
          for (size_t i = 0; i < state->samples_current; i++)
            fprintf(file, "%" PRIu32 ",", state->samples[i]);
          fclose(file);
        }
        state->samples_current = 0;
      }
      igEndDisabled();
    }
    igEnd();
  }

  if (state->anti_aliasing == AA_NONE)
  {
    aa_time_query_begin(&state->query);
    render_current_scene(state);
    aa_time_query_end(&state->query);
    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                             ? "aa_NONE.txt"
                                             : "aa_NONE_dartboard.txt";
  }

  if (state->anti_aliasing == AA_MSAAx4)
  {
    aa_time_query_begin(&state->query);
    aa_frame_buffer_bind(&state->msaa_fbo_x4);
    glClear(GL_COLOR_BUFFER_BIT);
    render_current_scene(state);
    // Blitting MSAA fbo to default fbo to render on screen
    aa_frame_buffer_blit(
        &state->default_fbo, &state->msaa_fbo_x4, state->window_width,
        state->window_height);
    aa_frame_buffer_bind(&state->default_fbo);
    aa_time_query_end(&state->query);

    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                             ? "aa_MSAAx4.txt"
                                             : "aa_MSAAx4_dartboard.txt";
  }

  if (state->anti_aliasing == AA_MSAAx8)
  {
    aa_time_query_begin(&state->query);
    // Bind MSAA framebuffer
    aa_frame_buffer_bind(&state->msaa_fbo_x8);
    glClear(GL_COLOR_BUFFER_BIT);
    render_current_scene(state);
    // Blit MSAA FBO to default framebuffer
    aa_frame_buffer_blit(
        &state->default_fbo, &state->msaa_fbo_x8, state->window_width,
        state->window_height);
    aa_frame_buffer_bind(&state->default_fbo);
    aa_time_query_end(&state->query);

    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                            ? "aa_MSAAx8.txt"
                                            : "aa_MSAAx8_dartboard.txt";
  }

  if (state->anti_aliasing == AA_MSAAx16)
  {
    aa_time_query_begin(&state->query);
    aa_frame_buffer_bind(&state->msaa_fbo_x16);
    glClear(GL_COLOR_BUFFER_BIT);
    render_current_scene(state);
    // Blit MSAA FBO to default framebuffer
    aa_frame_buffer_blit(
        &state->default_fbo, &state->msaa_fbo_x16, state->window_width,
        state->window_height);
    aa_frame_buffer_bind(&state->default_fbo);
    aa_time_query_end(&state->query);

    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                             ? "aa_MSAAx16.txt"
                                             : "aa_MSAAx16_dartboard.txt";
  }

  if (state->anti_aliasing == AA_FXAA)
  {
    aa_time_query_begin(&state->query);
    // Bind FXAA framebuffer
    aa_frame_buffer_bind(&state->fxaa_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    render_current_scene(state);
    // Post processing effects
    aa_frame_buffer_bind(&state->default_fbo);
    aa_program_use(&state->fxaa_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    // Don't need to rebind vbo
    glCall(glActiveTexture(GL_TEXTURE0));
    aa_texture_bind(&state->fxaa_color_texture);

    glUniform1i(glGetUniformLocation(state->fxaa_program.id, "screenTexture"), 0);
    glUniform2f(
        glGetUniformLocation(state->fxaa_program.id, "resolution"),
        state->window_width, state->window_height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    aa_time_query_end(&state->query);

    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                             ? "aa_FXAA.txt"
                                             : "aa_FXAA_dartboard.txt";
  }

  if (state->anti_aliasing == AA_FXAA_ITERATIVE)
  {
    aa_time_query_begin(&state->query);
    aa_frame_buffer_bind(&state->fxaa_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    render_current_scene(state);   
    // Post processing effects
    aa_frame_buffer_bind(&state->default_fbo);
    aa_program_use(&state->fxaa_iterative_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    aa_texture_bind(&state->fxaa_color_texture);

    glUniform1i(
        glGetUniformLocation(state->fxaa_iterative_program.id, "screenTexture"), 0);
    glUniform2f(
        glGetUniformLocation(state->fxaa_iterative_program.id, "resolution"),
        state->window_width, state->window_height);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    aa_time_query_end(&state->query);

    state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                             ? "aa_FXAA_Iterative.txt"
                                             : "aa_FXAA_Iterative_dartboard.txt";
  }

  // Check if current mode is any of the SMAA modes
  if (state->anti_aliasing >= AA_SMAA_LOW && state->anti_aliasing <= AA_SMAA_ULTRA)
  {
    aa_smaa_pipeline* smaa_pipeline = NULL;

    // Select the correct pipeline struct and log filename
    switch (state->anti_aliasing)
    {
    case AA_SMAA_LOW:
      smaa_pipeline = &state->smaa_low;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE) 
                                           ? "aa_SMAA_Low.txt" : "aa_SMAA_Low_dartboard.txt";
      break;
    case AA_SMAA_MEDIUM:
      smaa_pipeline = &state->smaa_medium;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE) 
                                           ? "aa_SMAA_Medium.txt" : "aa_SMAA_Medium_dartboard.txt";
      break;
    case AA_SMAA_HIGH:
      smaa_pipeline = &state->smaa_high;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE) 
                                           ? "aa_SMAA_High.txt" : "aa_SMAA_High_dartboard.txt";
      break;
    case AA_SMAA_ULTRA:
      smaa_pipeline = &state->smaa_ultra;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE) 
                                           ? "aa_SMAA_Ultra.txt" : "aa_SMAA_Ultra_dartboard.txt";
      break;
    default:
      break;
    }

    if (smaa_pipeline)
    {
      aa_time_query_begin(&state->query);
      aa_frame_buffer_bind(&state->smaa_fbo);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      render_current_scene(state);
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

      // Metrics required by SMAA.hlsl
      float w          = (float)state->window_width;
      float h          = (float)state->window_height;
      float metrics[4] = {1.0f / w, 1.0f / h, w, h};

      // Edge Detection Pass
      aa_frame_buffer_bind(&state->smaa_edge_fbo);
      glClear(GL_COLOR_BUFFER_BIT); 
      aa_program_use(&smaa_pipeline->edge_program);
      aa_vertex_array_bind(&state->fullscreen_vao);
      glActiveTexture(GL_TEXTURE0);
      aa_texture_bind(&state->smaa_color_texture);
      glUniform4fv(
          glGetUniformLocation(smaa_pipeline->edge_program.id, "SMAA_RT_METRICS"), 1,
          metrics);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->edge_program.id, "sceneTex"), 0);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // Blend Weight Pass
      aa_frame_buffer_bind(&state->smaa_blend_fbo);
      glClear(GL_COLOR_BUFFER_BIT);
      aa_program_use(&smaa_pipeline->blend_program);
      glActiveTexture(GL_TEXTURE0);
      aa_texture_bind(&state->smaa_edge_texture);
      glActiveTexture(GL_TEXTURE1);
      aa_texture_bind(&state->smaa_area_texture);
      glActiveTexture(GL_TEXTURE2);
      aa_texture_bind(&state->smaa_search_texture);
      glUniform4fv(
          glGetUniformLocation(smaa_pipeline->blend_program.id, "SMAA_RT_METRICS"), 1,
          metrics);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->blend_program.id, "edgeTex"), 0);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->blend_program.id, "areaTex"), 1);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->blend_program.id, "searchTex"), 2);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // Neighborhood Blending Pass (To Screen)
      aa_frame_buffer_bind(&state->default_fbo);
      aa_program_use(&smaa_pipeline->neighborhood_program);
      aa_vertex_array_bind(&state->fullscreen_vao);
      glActiveTexture(GL_TEXTURE0);
      aa_texture_bind(&state->smaa_color_texture);
      glActiveTexture(GL_TEXTURE1);
      aa_texture_bind(&state->smaa_blend_texture);
      glUniform4fv(
          glGetUniformLocation(smaa_pipeline->neighborhood_program.id, "SMAA_RT_METRICS"), 1,
          metrics);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->neighborhood_program.id, "sceneTex"),
          0);
      glUniform1i(
          glGetUniformLocation(smaa_pipeline->neighborhood_program.id, "blendTex"),
          1);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      aa_time_query_end(&state->query);
    }
  }

  aa_time_query_result(&state->query);
  // write in buffer
  if (state->samples_current < state->samples_total && state->is_recording)
  {
    state->samples[state->samples_current++] = state->query.result;
    if (state->samples_current == state->samples_total)
      if (!state->automation_mode)
        state->is_recording = false;
  }
}

// Function called once at the beginning 
static int on_init(AppState* state)
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
// Function called once after exiting loop
static void on_end(AppState* state)
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

  dartboard_cleanup(&state->dartboard);
}

// Function called when window gets resized
static void on_resize(AppState* state)
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

/// @brief Main loop of the application
/// @param window The window of the application (not null)
static void main_loop(
    GLFWwindow* window, ImGuiContext* context, ImGuiIO* io, int argc, char** argv)
{
  assert(window != NULL && "expected non-null window");

  AppState state;
  memset(&state, 0, sizeof(AppState));
  *(GLFWwindow**)(&state.window)          = window;
  *(ImGuiContext**)(&state.imgui_context) = context;
  *(ImGuiIO**)(&state.imgui_io)           = io;
  state.default_fbo.id                    = 0;

  // Default settings
  state.anti_aliasing                     = AA_NONE;
  state.automation_mode                   = false;
  state.warmup_frames                     = 0;
  state.current_scene                     = SCENE_TRIANGLE;

  // Handling automation runs from matlab
  if (argc > 1 && strcmp(argv[1], "--auto") == 0)
  {
    state.automation_mode = true;
    state.warmup_frames   = 100;
    // Nearly 8 seconds per algorithm
    AA_SAMPLE_COUNT       = 500;     
    printf("Running in Automation Mode (%d samples)\n", AA_SAMPLE_COUNT);
  }

  ImFontAtlas* atlas                      = io->Fonts;
  io->FontDefault                         = ImFontAtlas_AddFontFromFileTTF(
      atlas, "resources/Inter-4.1/InterVariable.ttf", 18.0f, NULL, NULL);
  glfwGetFramebufferSize(state.window, &state.window_width, &state.window_height);
  if (state.window_height < 32)
    state.window_height = 32;
  if (state.window_width < 32)
    state.window_width = 32;

  // ^^^ state setup
  if (on_init(&state) != 0)
    return;

  double last_time = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    double current_time = glfwGetTime();
    state.delta_time    = current_time - last_time;
    last_time           = current_time;
    state.elapsed_time += state.delta_time;

    glfwPollEvents();
    // resize viewport only on changes
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    // Set minimum fbo size to avoid 0 size
    if (height < 32)
      height = 32;
    if (width < 32)
      width = 32;
    if (width != state.window_width || height != state.window_height)
    {
      state.window_width  = width;
      state.window_height = height;
      glViewport(0, 0, width, height);
      on_resize(&state);
    }
    aa_frame_buffer_bind(&state.default_fbo);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
    // BEGIN FRAME:

    on_frame(&state);

    // END FRAME:
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      igUpdatePlatformWindows();
      igRenderPlatformWindowsDefault(NULL, NULL);
      glfwMakeContextCurrent(backup_current_context);
    }
    glfwSwapBuffers(window);
    ++state.frame_count;
  }
  on_end(&state);
}

/// @brief Application starting point
/// @return Exit code
int main(int argc, char** argv)
{
  printf(
      "Hello AA! (GLFW %i.%i.%i)\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR,
      GLFW_VERSION_REVISION);

  // initialize GLFW
  if (glfwInit() != GLFW_TRUE)
  {
    fputs("ERROR: Could not initialize GLFW!", stderr);
    exit(-1);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // create window, and OpenGL context
  GLFWwindow* window = glfwCreateWindow(640, 480, "aa - benchmarker", NULL, NULL);
  if (window == NULL)
  {
    fputs("ERROR: Could not create window!", stderr);
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(window);

  // initialize glad
  if (gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress) == 0)
  {
    fputs("ERROR: Could not initialize GLAD!", stderr);
    exit(-1);
  }

  // disable VSYNC
  glfwSwapInterval(0);

  // start ImGUI
  ImGuiContext* context = igCreateContext(NULL);
  ImGuiIO* io           = igGetIO_ContextPtr(context);
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  main_loop(window, context, io, argc, argv);

  // shutdown ImGUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(NULL);

  // destroy window
  glfwDestroyWindow(window);
  // shutdown GLFW
  glfwTerminate();
}