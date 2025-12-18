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

#include "appstate.h"
#include "lifecycle.h"
#include "ui_manual.h"
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

uint32_t AA_SAMPLE_COUNT = 100;

/// @brief Function handling draw calls, based on current scene to render
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

/// @brief In case running in automation mode, takes care of all the sampling logic for every algorithm and scene, then closes application
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

/// @brief Executes the rendering logic for a single frame
/// @details This function orchestrates the entire frame pipeline:
///          - Runs automation logic (only if enabled)
///          - Renders the ImGui control interface (in manual mode)
///          - Executes the selected Anti-Aliasing pipeline
///          - Measures GPU execution time using time queries
///          - Records samples
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
    // Setting up the UI control window (only in manual mode)
    aa_ui_render(state);
  }

  // Rendering Pipelines (Varying depending on chosen AA algorithm)
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
    glCall(glActiveTexture(GL_TEXTURE0));
    aa_texture_bind(&state->fxaa_color_texture);
    // Uniforms are variables that pass read-only data from the CPU to the GPU
    // for the duration of the current draw call
    glUniform1i(glGetUniformLocation(state->fxaa_program.id, "screenTexture"), 0);
    glUniform2f(
        glGetUniformLocation(state->fxaa_program.id, "resolution"),
        (float)state->window_width, (float)state->window_height);
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
        (float)state->window_width, (float)state->window_height);

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

    // Select the correct SMAA pipeline struct and log filename
    switch (state->anti_aliasing)
    {
    case AA_SMAA_LOW:
      smaa_pipeline                      = &state->smaa_low;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                               ? "aa_SMAA_Low.txt"
                                               : "aa_SMAA_Low_dartboard.txt";
      break;
    case AA_SMAA_MEDIUM:
      smaa_pipeline                      = &state->smaa_medium;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                               ? "aa_SMAA_Medium.txt"
                                               : "aa_SMAA_Medium_dartboard.txt";
      break;
    case AA_SMAA_HIGH:
      smaa_pipeline                      = &state->smaa_high;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                               ? "aa_SMAA_High.txt"
                                               : "aa_SMAA_High_dartboard.txt";
      break;
    case AA_SMAA_ULTRA:
      smaa_pipeline                      = &state->smaa_ultra;
      state->current_algorithm_file_name = (state->current_scene == SCENE_TRIANGLE)
                                               ? "aa_SMAA_Ultra.txt"
                                               : "aa_SMAA_Ultra_dartboard.txt";
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
          glGetUniformLocation(smaa_pipeline->blend_program.id, "SMAA_RT_METRICS"),
          1, metrics);
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
          glGetUniformLocation(
              smaa_pipeline->neighborhood_program.id, "SMAA_RT_METRICS"),
          1, metrics);
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
  // Getting time elapsed between start and end of every frame's rendering pipeline
  aa_time_query_result(&state->query);
  // Write in buffer
  if (state->samples_current < state->samples_total && state->is_recording)
  {
    state->samples[state->samples_current++] = state->query.result;
    if (state->samples_current == state->samples_total)
      if (!state->automation_mode)
        state->is_recording = false;
  }
}

/// @brief Main loop of the application
/// @param window Pointer to the active GLFW window (not null)
/// @param context Pointer to the ImGui context
/// @param io Pointer to the ImGui IO interface
/// @param argc Command line argument count
/// @param argv Command line argument values
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
  state.anti_aliasing   = AA_NONE;
  state.automation_mode = false;
  state.warmup_frames   = 0;
  state.current_scene   = SCENE_TRIANGLE;

  // Handling automation runs from matlab
  if (argc > 1 && strcmp(argv[1], "--auto") == 0)
  {
    state.automation_mode = true;
    state.warmup_frames   = 100;
    // Nearly 8 seconds per algorithm
    AA_SAMPLE_COUNT = 500;
    printf("Running in Automation Mode (%d samples)\n", AA_SAMPLE_COUNT);
  }

  ImFontAtlas* atlas = io->Fonts;
  io->FontDefault    = ImFontAtlas_AddFontFromFileTTF(
      atlas, "resources/Inter-4.1/InterVariable.ttf", 18.0f, NULL, NULL);
  glfwGetFramebufferSize(state.window, &state.window_width, &state.window_height);
  if (state.window_height < 32)
    state.window_height = 32;
  if (state.window_width < 32)
    state.window_width = 32;
  // Make sure all required data will be initialized successfully
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
/// @param argc Number of command-line arguments
/// @param argv Array of command-line argument strings
/// @return Exit code
int main(int argc, char** argv)
{
  printf(
      "Hello AA! (GLFW %i.%i.%i)\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR,
      GLFW_VERSION_REVISION);

  // Initialize GLFW
  if (glfwInit() != GLFW_TRUE)
  {
    fputs("ERROR: Could not initialize GLFW!", stderr);
    exit(-1);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // Create window, and OpenGL context
  GLFWwindow* window = glfwCreateWindow(640, 480, "aa - benchmarker", NULL, NULL);
  if (window == NULL)
  {
    fputs("ERROR: Could not create window!", stderr);
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(window);

  // Initialize glad
  if (gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress) == 0)
  {
    fputs("ERROR: Could not initialize GLAD!", stderr);
    exit(-1);
  }

  // Disable VSYNC
  glfwSwapInterval(0);

  // Start ImGUI
  ImGuiContext* context = igCreateContext(NULL);
  ImGuiIO* io           = igGetIO_ContextPtr(context);
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  main_loop(window, context, io, argc, argv);

  // Shutdown ImGUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(NULL);

  // Destroy window
  glfwDestroyWindow(window);
  // Shutdown GLFW
  glfwTerminate();
}