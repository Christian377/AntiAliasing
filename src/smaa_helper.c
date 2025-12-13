#include "smaa_helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glad/glad.h>

char* aa_concat_smaa_source(
    const char* preamble, const char* smaa_lib, const char* shader_logic)
{
  if (!preamble || !smaa_lib || !shader_logic)
    return NULL;

  size_t len_pre = strlen(preamble);
  size_t len_lib = strlen(smaa_lib);
  size_t len_log = strlen(shader_logic);

  char* result = (char*)malloc(len_pre + len_lib + len_log + 1);
  if (!result)
    return NULL;

  memcpy(result, preamble, len_pre);
  memcpy(result + len_pre, smaa_lib, len_lib);
  memcpy(result + len_pre + len_lib, shader_logic, len_log);
  result[len_pre + len_lib + len_log] = '\0';

  return result;
}

//int aa_smaa_pipeline_init(
//    aa_smaa_pipeline* p, const char* preset_macro, const char* lib_src,
//    const char* vs_edge_raw, const char* fs_edge_raw, const char* vs_blend_raw,
//    const char* fs_blend_raw, const char* vs_neigh_raw, const char* fs_neigh_raw)
//{
//  const char* base_header = "#version 430 core\n"
//                            "#define SMAA_GLSL_4 1\n"
//                            "uniform vec4 SMAA_RT_METRICS;\n";
//
//  // Combine base header + preset macro
//  size_t header_len = strlen(base_header) + strlen(preset_macro) + 1;
//  char* full_header = malloc(header_len);
//  if (!full_header)
//    return -1;
//  strcpy(full_header, base_header);
//  strcat(full_header, preset_macro);
//
//  // Concat Sources
//  char* src_e_vs = aa_concat_smaa_source(full_header, lib_src, vs_edge_raw);
//  char* src_e_fs = aa_concat_smaa_source(full_header, lib_src, fs_edge_raw);
//  char* src_b_vs = aa_concat_smaa_source(full_header, lib_src, vs_blend_raw);
//  char* src_b_fs = aa_concat_smaa_source(full_header, lib_src, fs_blend_raw);
//  char* src_n_vs = aa_concat_smaa_source(full_header, lib_src, vs_neigh_raw);
//  char* src_n_fs = aa_concat_smaa_source(full_header, lib_src, fs_neigh_raw);
//
//  free(full_header);
//
//  if (!src_e_vs || !src_e_fs || !src_b_vs || !src_b_fs || !src_n_vs || !src_n_fs)
//  {
//    return -1;
//  }
//
//  // Compile Shaders
//  aa_vertex_shader_create(&p->edge_vs, src_e_vs);
//  aa_vertex_shader_compile(&p->edge_vs);
//  aa_fragment_shader_create(&p->edge_fs, src_e_fs);
//  aa_fragment_shader_compile(&p->edge_fs);
//
//  aa_vertex_shader_create(&p->blend_vs, src_b_vs);
//  aa_vertex_shader_compile(&p->blend_vs);
//  aa_fragment_shader_create(&p->blend_fs, src_b_fs);
//  aa_fragment_shader_compile(&p->blend_fs);
//
//  aa_vertex_shader_create(&p->neigh_vs, src_n_vs);
//  aa_vertex_shader_compile(&p->neigh_vs);
//  aa_fragment_shader_create(&p->neigh_fs, src_n_fs);
//  aa_fragment_shader_compile(&p->neigh_fs);
//
//  // Link Programs
//  aa_program_create(&p->edge_program);
//  aa_program_attach_shaders(&p->edge_program, &p->edge_vs, &p->edge_fs);
//  aa_program_link(&p->edge_program);
//
//  aa_program_create(&p->blend_program);
//  aa_program_attach_shaders(&p->blend_program, &p->blend_vs, &p->blend_fs);
//  aa_program_link(&p->blend_program);
//
//  aa_program_create(&p->neighborhood_program);
//  aa_program_attach_shaders(&p->neighborhood_program, &p->neigh_vs, &p->neigh_fs);
//  aa_program_link(&p->neighborhood_program);
//
//  // Cleanup Source Strings
//  free(src_e_vs);
//  free(src_e_fs);
//  free(src_b_vs);
//  free(src_b_fs);
//  free(src_n_vs);
//  free(src_n_fs);
//
//  return 0;
//}

int aa_smaa_pipeline_init(
    aa_smaa_pipeline* p, const char* preset_macro, const char* lib_src,
    const char* vs_edge_raw, const char* fs_edge_raw, const char* vs_blend_raw,
    const char* fs_blend_raw, const char* vs_neigh_raw, const char* fs_neigh_raw)
{
  // 1. Base Header: Version + Uniforms + Defines
  const char* base_string = "#version 430 core\n"
                            "#define SMAA_GLSL_4 1\n"
                            "uniform vec4 SMAA_RT_METRICS;\n";

  // 2. Fragment Shader Header: Standard
  char* full_header = aa_concat_smaa_source(base_string, preset_macro, "");

  // 3. Vertex Shader Header: THE NUCLEAR FIX
  // We define "discard" to be empty space. This tricks the Vertex Shader
  // into ignoring the discard commands inside the SMAA library.
  const char* no_discard_define = "#define SMAA_DISCARD \n"
                                  "#define discard \n";

  char* temp_header = aa_concat_smaa_source(base_string, preset_macro, "");
  char* vs_header   = aa_concat_smaa_source(temp_header, no_discard_define, "");
  free(temp_header);

  if (!full_header || !vs_header)
    return -1;

  // 4. Concatenate Sources
  // Use vs_header for Vertex Shaders, full_header for Fragment Shaders
  char* src_e_vs = aa_concat_smaa_source(vs_header, lib_src, vs_edge_raw);
  char* src_e_fs = aa_concat_smaa_source(full_header, lib_src, fs_edge_raw);

  char* src_b_vs = aa_concat_smaa_source(vs_header, lib_src, vs_blend_raw);
  char* src_b_fs = aa_concat_smaa_source(full_header, lib_src, fs_blend_raw);

  char* src_n_vs = aa_concat_smaa_source(vs_header, lib_src, vs_neigh_raw);
  char* src_n_fs = aa_concat_smaa_source(full_header, lib_src, fs_neigh_raw);

  // Clean up headers
  free(full_header);
  free(vs_header);

  if (!src_e_vs || !src_e_fs || !src_b_vs || !src_b_fs || !src_n_vs || !src_n_fs)
  {
    return -1;
  }

  // 5. Compile Shaders
  aa_vertex_shader_create(&p->edge_vs, src_e_vs);
  aa_vertex_shader_compile(&p->edge_vs);
  aa_fragment_shader_create(&p->edge_fs, src_e_fs);
  aa_fragment_shader_compile(&p->edge_fs);

  aa_vertex_shader_create(&p->blend_vs, src_b_vs);
  aa_vertex_shader_compile(&p->blend_vs);
  aa_fragment_shader_create(&p->blend_fs, src_b_fs);
  aa_fragment_shader_compile(&p->blend_fs);

  aa_vertex_shader_create(&p->neigh_vs, src_n_vs);
  aa_vertex_shader_compile(&p->neigh_vs);
  aa_fragment_shader_create(&p->neigh_fs, src_n_fs);
  aa_fragment_shader_compile(&p->neigh_fs);

  // 6. Link Programs
  aa_program_create(&p->edge_program);
  aa_program_attach_shaders(&p->edge_program, &p->edge_vs, &p->edge_fs);
  aa_program_link(&p->edge_program);

  aa_program_create(&p->blend_program);
  aa_program_attach_shaders(&p->blend_program, &p->blend_vs, &p->blend_fs);
  aa_program_link(&p->blend_program);

  aa_program_create(&p->neighborhood_program);
  aa_program_attach_shaders(&p->neighborhood_program, &p->neigh_vs, &p->neigh_fs);
  aa_program_link(&p->neighborhood_program);

  // 7. Cleanup Source Strings
  free(src_e_vs);
  free(src_e_fs);
  free(src_b_vs);
  free(src_b_fs);
  free(src_n_vs);
  free(src_n_fs);

  return 0;
}

void aa_smaa_pipeline_delete(aa_smaa_pipeline* p)
{
  aa_program_delete(&p->edge_program);
  aa_program_delete(&p->blend_program);
  aa_program_delete(&p->neighborhood_program);

  aa_vertex_shader_delete(&p->edge_vs);
  aa_fragment_shader_delete(&p->edge_fs);
  aa_vertex_shader_delete(&p->blend_vs);
  aa_fragment_shader_delete(&p->blend_fs);
  aa_vertex_shader_delete(&p->neigh_vs);
  aa_fragment_shader_delete(&p->neigh_fs);
}


