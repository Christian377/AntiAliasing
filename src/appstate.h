#ifndef __AA_HG_APPSTATE
#define __AA_HG_APPSTATE

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include <cimgui.h>
#include <cimgui_impl.h>
#include "gl/frame_buffer.h"
#include "gl/program.h"
#include "gl/vertex_array.h"
#include "gl/vertex_buffer.h"
#include "gl/shaders.h"
#include "gl/query.h"
#include "dartboard.h"
#include "smaa_helper.h"

/// @brief The number of samples to try to capture
extern uint32_t AA_SAMPLE_COUNT;

/// @brief Anti Aliasing Options
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

/// @brief The scene to draw
typedef enum
{
  SCENE_TRIANGLE,
  SCENE_DARTBOARD
} SceneType;

/// @brief Application state, across frames
typedef struct
{
  // The window (never null)
  GLFWwindow* const window;
  // The ImGUI context (never null)
  ImGuiContext* const imgui_context;
  // The ImGUI IO of `imgui_context` (never null)
  ImGuiIO* const imgui_io;
  // The window's width
  int window_width;
  // The window's height
  int window_height;
  // The frame count
  uint64_t frame_count;
  // The delta time between frames (in seconds)
  double delta_time;
  // The time passed from first frame until present
  double elapsed_time;
  // The currently used anti aliasing algorithm
  aa_algorithm anti_aliasing;
  // The time query responsible for measuring algorithms performance
  aa_time_query query;
  // The vertex array object for position only vertices
  aa_vertex_array vao;
  // The position only vertex buffer object
  aa_vertex_buffer vbo;
  // The veryex array object for position and uv attributes vertices
  aa_vertex_array fullscreen_vao;
  // The position and uv attributes vertex buffer object
  aa_vertex_buffer fullscreen_vbo;
  // Program used to draw the scene without any specific additional effect
  aa_program program;
  // Program used to apply FXAA
  aa_program fxaa_program;
  // Program used to apply FXAA
  aa_program fxaa_iterative_program;
  // Vertex shader which takes as input position only vertices
  aa_fragment_shader default_fragment_shader;
  // Fragment shader which simply draws vertex shader outputs without any additional effect
  aa_vertex_shader default_vertex_shader;
  // Fragment shader containing FXAA post processing algorithm
  aa_fragment_shader fxaa_fragment_shader;
  // Fragment shader containing FXAA post processing algorithm
  aa_fragment_shader fxaa_iterative_fragment_shader;
  // Vertex shader used to render a texture on the screen
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
  // The scene to be drawn
  SceneType current_scene;
  // The dartboard scene data
  DartboardScene dartboard;
} AppState;

#endif // !__AA_HG_APPSTATE
