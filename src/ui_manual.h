#ifndef __AA_HG_MANUAL_UI
#define __AA_HG_MANUAL_UI

#include "appstate.h"

/// @brief Renders the ImGui control panel and handles user interaction
/// @details Defines the UI overlay layout, including buttons for algorithm switching,
///          scene selection, and recording controls. Modifies the AppState directly based on user input.
/// @param state The application state
void aa_ui_render(AppState* state);

#endif // !__AA_HG_MANUAL_UI
