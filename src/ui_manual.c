#include "ui_manual.h"

void aa_ui_render(AppState* state)
{
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
