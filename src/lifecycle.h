#ifndef __AA_HG_LIFECYCLE
#define __AA_HG_LIFECYCLE

#include <glad/glad.h>
#include "smaa/AreaTex.h"
#include "smaa/SearchTex.h"
#include "appstate.h"
#include "dartboard.h"
#include "smaa_helper.h"

/// @brief Initializes the application state, OpenGL resources, and scene data
/// @details This function is responsible for:
///          - Compiling and linking all shader programs
///          - Initializing the SMAA pipeline with pre-computed textures
///          - Creating Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs)
///          - Setting up Framebuffer Objects (FBOs) and backing textures for off-screen rendering
///          - Initializing the specific scene geometry
/// @param state The application state structure to be populated
/// @return 0 on success, -1 if any shader compilation or resource allocation fails
int on_init(AppState* state);

/// @brief Function called when window gets resized
void on_resize(AppState* state);

/// @brief cleans up all allocated resources before application exit.
void on_end(AppState* state);

#endif // !__AA_HG_LIFECYCLE
