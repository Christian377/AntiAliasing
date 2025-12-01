#ifndef __HG_AA_GL_QUERY
#define __HG_AA_GL_QUERY

#include <glad/glad.h>
#include <stdint.h>
#include "error.h"

typedef struct
{
  unsigned int id;
  uint64_t result;

}aa_time_query;

void aa_time_query_create(aa_time_query* out);
void aa_time_query_delete(aa_time_query* out);
void aa_time_query_begin(aa_time_query* out);
void aa_time_query_end(aa_time_query* out);
void aa_time_query_result(aa_time_query* out);

#endif // !__HG_AA_GL_QUERY
