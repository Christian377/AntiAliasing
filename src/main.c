#include <stdbool.h>
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


/// @brief Application state, across frames
typedef struct
{
  /// @brief The window (never null)
  GLFWwindow* const window;
  /// @brief The ImGUI context (never null)
  ImGuiContext* const imgui_context;
  /// @brief The ImGUI IO of `imgui_context` (never null)
  ImGuiIO* const imgui_io;

  /// @brief The frame count
  uint64_t frame_count;
  /// @brief The delta time between frames (in seconds)
  double delta_time;
  /// @brief The time passed from first frame until present
  double elapsed_time;
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
    igShowDemoWindow(NULL);
  }
  igEnd();
}

/// @brief Main loop of the application
/// @param window The window of the application (not null)
static void main_loop(GLFWwindow* window, ImGuiContext* context, ImGuiIO* io)
{
  assert(window != NULL && "expected non-null window");

  AppState state;
  memset(&state, 0, sizeof(AppState));
  *(GLFWwindow**)(&state.window) = window;
  *(ImGuiContext**)(&state.imgui_context) = context;
  *(ImGuiIO**)(&state.imgui_io) = io;

  double last_time = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    double current_time = glfwGetTime();
    state.delta_time    = current_time - last_time;
    last_time           = current_time;
    state.elapsed_time += state.delta_time;

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    on_frame(&state);

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

  // enable VSYNC
  glfwSwapInterval(1);

  // start ImGUI
  ImGuiContext* context = igCreateContext(NULL);
  ImGuiIO* io = igGetIO_ContextPtr(context);
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