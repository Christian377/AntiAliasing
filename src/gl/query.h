#ifndef __HG_AA_GL_QUERY
#define __HG_AA_GL_QUERY

#include <glad/glad.h>
#include <stdint.h>
#include "error.h"

// Wrappers for OpenGL time queries
// Provides functionality to measure GPU execution time (in nanoseconds)
// for performance profiling and benchmarking of render passes

typedef struct
{
  unsigned int id;
  uint32_t result;
} aa_time_query;

void aa_time_query_create(aa_time_query* out);
void aa_time_query_delete(aa_time_query* out);
void aa_time_query_begin(aa_time_query* out);
void aa_time_query_end(aa_time_query* out);
void aa_time_query_result(aa_time_query* out);

#endif // !__HG_AA_GL_QUERY
