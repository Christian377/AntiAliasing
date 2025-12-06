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

#include "stb_image/stb_image.h"
#include "gl/program.h"
#include "gl/shaders.h"
#include "gl/vertex_array.h"
#include "gl/vertex_buffer.h"
#include "gl/frame_buffer.h"
#include "gl/query.h"
#include "smaa/AreaTex.h"
#include "smaa/SearchTex.h"

__declspec(dllexport) uint32_t NvOptimusEnablement             = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef enum
{
  AA_NONE,
  AA_MSAAx8,
  AA_FXAA,
  AA_SMAA
} aa_algorithm;

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
  /// @brief Program used in SMAA edge detection pass
  aa_program smaa_edge_program;
  /// @brief Program used in SMAA blend weight calculation pass
  aa_program smaa_blend_program;
  /// @brief Program used in SMAA neighborhood blending pass
  aa_program smaa_neighborhood_program;
  /// @brief Vertex shader which takes as input position only vertices
  aa_fragment_shader default_fragment_shader;
  /// @brief Fragment shader which simply draws vertex shader outputs without any additional effect
  aa_vertex_shader default_vertex_shader;
  /// @brief Fragment shader containing FXAA post processing algorithm
  aa_fragment_shader fxaa_fragment_shader;
  /// @brief Vertex shader used to render a texture on the screen
  aa_vertex_shader fullscreen_quad_vertex_shader;
  // SMAA fragment shaders
  aa_fragment_shader smaa_edge_fragment_shader;
  aa_fragment_shader smaa_blend_fragment_shader;
  aa_fragment_shader smaa_neighborhood_fragment_shader;
  // Default fbo, with id 0
  aa_frame_buffer default_fbo;
  // MSAA multisampling fbo and texture
  aa_frame_buffer msaa_fbo;
  aa_texture msaa_color_texture;
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
} AppState;

/// @brief Function called once per frame
/// @param state The application state
static void on_frame(AppState* state)
{
  glClearColor(
      fabsf(sinf((float)state->elapsed_time * 1.4f)),
      fabsf(sinf((float)state->elapsed_time * 1.1f)),
      fabsf(sinf((float)state->elapsed_time * 0.8f)), 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (igBegin("Hello World", NULL, 0))
  {
    igText("Hello World from SIE!");
    if (igButton("No AA", (ImVec2){0, 0}))
      state->anti_aliasing = AA_NONE;
    if (igButton("MSAA", (ImVec2){0, 0}))
      state->anti_aliasing = AA_MSAAx8;
    if (igButton("FXAA", (ImVec2){0, 0}))
      state->anti_aliasing = AA_FXAA;
    if (igButton("SMAA", (ImVec2){0, 0}))
      state->anti_aliasing = AA_SMAA;
  }
  igEnd();

  if (state->anti_aliasing == AA_NONE)
  {
    aa_time_query_begin(&state->query);
    aa_program_use(&state->program);
    aa_vertex_array_bind(&state->vao);
    aa_vertex_buffer_bind(&state->vbo);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    aa_time_query_end(&state->query);
  }

  if (state->anti_aliasing == AA_MSAAx8)
  {
    aa_time_query_begin(&state->query);
    // Bind MSAA framebuffer
    aa_frame_buffer_bind(&state->msaa_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    aa_program_use(&state->program);
    aa_vertex_array_bind(&state->vao);
    aa_vertex_buffer_bind(&state->vbo);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Blit MSAA FBO to default framebuffer
    aa_frame_buffer_blit(
        &state->default_fbo, &state->msaa_fbo, state->window_width,
        state->window_height);
    aa_frame_buffer_bind(&state->default_fbo);
    aa_time_query_end(&state->query);
  }

  if (state->anti_aliasing == AA_FXAA)
  {
    aa_time_query_begin(&state->query);
    // Bind FXAA framebuffer
    aa_frame_buffer_bind(&state->fxaa_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    aa_program_use(&state->program);
    aa_vertex_array_bind(&state->vao);
    aa_vertex_buffer_bind(&state->vbo);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    aa_frame_buffer_bind(&state->default_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    aa_program_use(&state->fxaa_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    //aa_vertex_buffer_bind(&fullscreen_vbo);

    //dont need to rebind vbo(careful quand meme)
    glCall(glActiveTexture(GL_TEXTURE0));
    aa_texture_bind(&state->fxaa_color_texture);

    glUniform1i(glGetUniformLocation(state->fxaa_program.id, "screenTexture"), 0);
    glUniform2f(
        glGetUniformLocation(state->fxaa_program.id, "resolution"),
        state->window_width, state->window_height);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    aa_time_query_end(&state->query);
  }

  if (state->anti_aliasing == AA_SMAA)
  {
    aa_time_query_begin(&state->query);
    aa_frame_buffer_bind(&state->smaa_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    aa_program_use(&state->program);
    aa_vertex_array_bind(&state->vao);
    aa_vertex_buffer_bind(&state->vbo);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Edge detection
    aa_frame_buffer_bind(&state->smaa_edge_fbo);
    aa_program_use(&state->smaa_edge_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    // bind color texture as input
    glActiveTexture(GL_TEXTURE0);
    aa_texture_bind(&state->smaa_color_texture);
    glUniform1i(glGetUniformLocation(state->smaa_edge_program.id, "sceneTex"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Blend weight calculation pass
    aa_frame_buffer_bind(&state->smaa_blend_fbo); // bind to blend weight texture
    aa_program_use(&state->smaa_blend_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    aa_texture_bind(&state->smaa_edge_texture);
    glActiveTexture(GL_TEXTURE1);
    aa_texture_bind(&state->smaa_area_texture);
    glActiveTexture(GL_TEXTURE2);
    aa_texture_bind(&state->smaa_search_texture);
    glUniform1i(glGetUniformLocation(state->smaa_blend_program.id, "edgeTex"), 0);
    glUniform1i(glGetUniformLocation(state->smaa_blend_program.id, "areaTex"), 1);
    glUniform1i(glGetUniformLocation(state->smaa_blend_program.id, "searchTex"), 2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Neighborhood blending pass
    aa_frame_buffer_bind(&state->default_fbo); // render final result to screen
    aa_program_use(&state->smaa_neighborhood_program);
    aa_vertex_array_bind(&state->fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    aa_texture_bind(&state->smaa_color_texture);
    glActiveTexture(GL_TEXTURE1);
    aa_texture_bind(&state->smaa_blend_texture);
    glUniform1i(
        glGetUniformLocation(state->smaa_neighborhood_program.id, "sceneTex"), 0);
    glUniform1i(
        glGetUniformLocation(state->smaa_neighborhood_program.id, "blendTex"), 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    aa_time_query_end(&state->query);
  }

  aa_time_query_result(&state->query);
  printf("%" PRIu32 "\n", state->query.result);
}

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

  char* VERTEX_DEFAULT   = aa_load_file("resources/shaders/vertex_default.glsl");
  char* FRAGMENT_DEFAULT = aa_load_file("resources/shaders/fragment_default.glsl");

  char* VERTEX_FULLSCREEN_QUAD =
      aa_load_file("resources/shaders/vertex_fullscreen_quad.glsl");
  char* FRAGMENT_FXAA = aa_load_file("resources/shaders/fragment_fxaa.glsl");
  char* FRAGMENT_EDGE_SMAA =
      aa_load_file("resources/shaders/fragment_edge_smaa.glsl");
  char* FRAGMENT_BLEND_SMAA =
      aa_load_file("resources/shaders/fragment_blend_smaa.glsl");
  char* FRAGMENT_NEIGHBORHOOD_SMAA =
      aa_load_file("resources/shaders/fragment_neighborhood_smaa.glsl");

  if (VERTEX_DEFAULT == NULL || FRAGMENT_DEFAULT == NULL
      || VERTEX_FULLSCREEN_QUAD == NULL || FRAGMENT_FXAA == NULL
      || FRAGMENT_EDGE_SMAA == NULL || FRAGMENT_BLEND_SMAA == NULL
      || FRAGMENT_NEIGHBORHOOD_SMAA == NULL)
  {
    free(VERTEX_DEFAULT);
    free(FRAGMENT_DEFAULT);
    free(VERTEX_FULLSCREEN_QUAD);
    free(FRAGMENT_FXAA);
    free(FRAGMENT_EDGE_SMAA);
    free(FRAGMENT_BLEND_SMAA);
    free(FRAGMENT_NEIGHBORHOOD_SMAA);
    return -1;
  }

  aa_time_query_create(&state->query);

  // create programs
  aa_program_create(&state->program);
  aa_program_create(&state->fxaa_program);
  aa_program_create(&state->smaa_edge_program);
  aa_program_create(&state->smaa_blend_program);
  aa_program_create(&state->smaa_neighborhood_program);

  // create shaders
  aa_vertex_shader_create(&state->default_vertex_shader, VERTEX_DEFAULT);
  aa_fragment_shader_create(&state->default_fragment_shader, FRAGMENT_DEFAULT);

  aa_vertex_shader_create(
      &state->fullscreen_quad_vertex_shader, VERTEX_FULLSCREEN_QUAD);
  aa_fragment_shader_create(&state->fxaa_fragment_shader, FRAGMENT_FXAA);

  aa_fragment_shader_create(&state->smaa_edge_fragment_shader, FRAGMENT_EDGE_SMAA);
  aa_fragment_shader_create(&state->smaa_blend_fragment_shader, FRAGMENT_BLEND_SMAA);
  aa_fragment_shader_create(
      &state->smaa_neighborhood_fragment_shader, FRAGMENT_NEIGHBORHOOD_SMAA);

  // compile shaders
  aa_vertex_shader_compile(&state->default_vertex_shader);
  aa_fragment_shader_compile(&state->default_fragment_shader);

  aa_vertex_shader_compile(&state->fullscreen_quad_vertex_shader);
  aa_fragment_shader_compile(&state->fxaa_fragment_shader);

  aa_fragment_shader_compile(&state->smaa_edge_fragment_shader);
  aa_fragment_shader_compile(&state->smaa_blend_fragment_shader);
  aa_fragment_shader_compile(&state->smaa_neighborhood_fragment_shader);

  // attach shaders andlink programs
  aa_program_attach_shaders(
      &state->fxaa_program, &state->fullscreen_quad_vertex_shader,
      &state->fxaa_fragment_shader);
  aa_program_link(&state->fxaa_program);

  aa_program_attach_shaders(
      &state->smaa_edge_program, &state->fullscreen_quad_vertex_shader,
      &state->smaa_edge_fragment_shader);
  aa_program_link(&state->smaa_edge_program);
  aa_program_attach_shaders(
      &state->smaa_blend_program, &state->fullscreen_quad_vertex_shader,
      &state->smaa_blend_fragment_shader);
  aa_program_link(&state->smaa_blend_program);
  aa_program_attach_shaders(
      &state->smaa_neighborhood_program, &state->fullscreen_quad_vertex_shader,
      &state->smaa_neighborhood_fragment_shader);
  aa_program_link(&state->smaa_neighborhood_program);

  aa_program_attach_shaders(
      &state->program, &state->default_vertex_shader,
      &state->default_fragment_shader);
  aa_program_link(&state->program);

  //triangle vao and vbo setup
  aa_vertex_buffer_create(&state->vbo);
  aa_vertex_buffer_update(&state->vbo, vertices, sizeof(vertices));

  aa_vertex_array_create(&state->vao);
  aa_vertex_array_position_attribute(&state->vao);

  //fullscreen quad vao and vbo setup
  aa_vertex_buffer_create(&state->fullscreen_vbo);
  aa_vertex_buffer_update(
      &state->fullscreen_vbo, fullscreen_vertices, sizeof(fullscreen_vertices));
  aa_vertex_array_create(&state->fullscreen_vao);
  aa_vertex_array_position_uv_attribute(&state->fullscreen_vao);

  glfwGetFramebufferSize(state->window, &state->window_width, &state->window_height);

  //MSAA multisampling fbo and texture initialization
  aa_frame_buffer_create(&state->msaa_fbo);
  aa_texture_msaa_create(&state->msaa_color_texture);
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture, state->window_width, state->window_height, 8);
  aa_frame_buffer_color_texture(&state->msaa_fbo, &state->msaa_color_texture);
  aa_frame_buffer_bind(&state->default_fbo);

  //FXAA fbo and screen texture initialization
  aa_frame_buffer_create(&state->fxaa_fbo);
  aa_texture_create(
      &state->fxaa_color_texture, state->window_width, state->window_height);
  aa_frame_buffer_color_texture(&state->fxaa_fbo, &state->fxaa_color_texture);

  //FXAA fbo and required textures initialization
  aa_frame_buffer_create(&state->smaa_fbo);
  aa_frame_buffer_create(&state->smaa_edge_fbo);
  aa_frame_buffer_create(&state->smaa_blend_fbo);
  aa_texture_from_data(&state->smaa_area_texture, areaTexBytes, 160, 560);
  aa_texture_from_data(&state->smaa_search_texture, searchTexBytes, 60, 33);
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
  free(FRAGMENT_EDGE_SMAA);
  free(FRAGMENT_BLEND_SMAA);
  free(FRAGMENT_NEIGHBORHOOD_SMAA);

  return 0;
}

static void on_end(AppState* state)
{
  // Delete Programs
  aa_program_delete(&state->program);
  aa_program_delete(&state->fxaa_program);
  aa_program_delete(&state->smaa_edge_program);
  aa_program_delete(&state->smaa_blend_program);
  aa_program_delete(&state->smaa_neighborhood_program);

  // Delete Shaders 
  aa_vertex_shader_delete(&state->default_vertex_shader);
  aa_fragment_shader_delete(&state->default_fragment_shader);
  aa_vertex_shader_delete(&state->fullscreen_quad_vertex_shader);
  aa_fragment_shader_delete(&state->fxaa_fragment_shader);
  aa_fragment_shader_delete(&state->smaa_edge_fragment_shader);
  aa_fragment_shader_delete(&state->smaa_blend_fragment_shader);
  aa_fragment_shader_delete(&state->smaa_neighborhood_fragment_shader);

  // 3. Delete Buffers and vaos
  aa_vertex_buffer_delete(&state->vbo);
  aa_vertex_array_delete(&state->vao);
  aa_vertex_buffer_delete(&state->fullscreen_vbo);
  aa_vertex_array_delete(&state->fullscreen_vao);

  // Delete Framebuffers
  aa_frame_buffer_delete(&state->msaa_fbo);
  aa_frame_buffer_delete(&state->fxaa_fbo);
  aa_frame_buffer_delete(&state->smaa_fbo);
  aa_frame_buffer_delete(&state->smaa_edge_fbo);
  aa_frame_buffer_delete(&state->smaa_blend_fbo);

  // Delete Textures
  aa_texture_delete(&state->msaa_color_texture);
  aa_texture_delete(&state->fxaa_color_texture);
  aa_texture_delete(&state->smaa_color_texture);
  aa_texture_delete(&state->smaa_area_texture);
  aa_texture_delete(&state->smaa_search_texture);
  aa_texture_delete(&state->smaa_edge_texture);
  aa_texture_delete(&state->smaa_blend_texture);

  // Delete Query
  aa_time_query_delete(&state->query);
}

static void on_resize(AppState* state)
{
  aa_texture_msaa_dimensions(
      &state->msaa_color_texture, state->window_width, state->window_height, 8);
  aa_frame_buffer_color_texture(&state->msaa_fbo, &state->msaa_color_texture);
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
static void main_loop(GLFWwindow* window, ImGuiContext* context, ImGuiIO* io)
{
  assert(window != NULL && "expected non-null window");

  AppState state;
  memset(&state, 0, sizeof(AppState));
  *(GLFWwindow**)(&state.window)          = window;
  *(ImGuiContext**)(&state.imgui_context) = context;
  *(ImGuiIO**)(&state.imgui_io)           = io;
  state.anti_aliasing                     = AA_NONE;
  state.default_fbo.id                    = 0;  
  ImFontAtlas* atlas = io->Fonts;
  io->FontDefault    = ImFontAtlas_AddFontFromFileTTF(
      atlas, "resources/Inter-4.1/InterVariable.ttf", 18.0f, NULL, NULL);
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
int main()
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

  main_loop(window, context, io);

  // shutdown ImGUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(NULL);

  // destroy window
  glfwDestroyWindow(window);
  // shutdown GLFW
  glfwTerminate();
}