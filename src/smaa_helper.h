#ifndef __AA_SMAA_SMAA_HELPER
#define __AA_SMAA_SMAA_HELPER

#include <stdlib.h>
#include <string.h>
#include "gl/program.h"
#include "gl/shaders.h"

typedef struct
{
  aa_program edge_program;
  aa_program blend_program;
  aa_program neighborhood_program;

  // Keeping shaders to delete them cleanly later
  aa_vertex_shader edge_vs, blend_vs, neigh_vs;
  aa_fragment_shader edge_fs, blend_fs, neigh_fs;
} aa_smaa_pipeline;


///@brief Initializes a specific SMAA pipeline (e.g. Low, Ultra)
///@param pipeline Pointer to the struct to initialize
///@param preset_macro The specific define string (e.g. "#define SMAA_PRESET_ULTRA 1\n")
///@param lib_src Content of SMAA.hlsl
///@param vs_edge_src Content of vertex_edge_smaa.glsl
///@param fs_edge_src Content of fragment_edge_smaa.glsl
///@param vs_blend_src Content of vertex_blend_smaa.glsl
///@param fs_blend_src Content of fragment_blend_smaa.glsl
///@param vs_neigh_src Content of vertex_neighborhood_smaa.glsl
///@param fs_neigh_src Content of fragment_neighborhood_smaa.glsl
///@return 0 on success, -1 on failure
int aa_smaa_pipeline_init(
    aa_smaa_pipeline* pipeline, const char* preset_macro, const char* lib_src,
    const char* vs_edge_src, const char* fs_edge_src, const char* vs_blend_src,
    const char* fs_blend_src, const char* vs_neigh_src, const char* fs_neigh_src);

// Cleans up the programs and shaders in the pipeline
void aa_smaa_pipeline_delete(aa_smaa_pipeline* pipeline);

// Helper to concatenate strings (Preamble + Lib + Body)
char* aa_concat_smaa_source(
    const char* preamble, const char* library, const char* body);

char* aa_concat_smaa_source(
    const char* preamble, const char* smaa_lib, const char* shader_logic);

#endif // !__AA_SMAA_SMAA_HELPER
