#include "query.h"

void aa_time_query_create(aa_time_query* out)
{
  glCall(glGenQueries(1, &out->id));
}

void aa_time_query_delete(aa_time_query* out)
{
  glCall(glDeleteQueries(1, &out->id));
}

void aa_time_query_begin(aa_time_query* out)
{
  glCall(glBeginQuery(GL_TIME_ELAPSED, out->id));
}

void aa_time_query_end(aa_time_query* out)
{
  glCall(glEndQuery(GL_TIME_ELAPSED));
}

void aa_time_query_result(aa_time_query* out)
{
  glCall(glGetQueryObjectuiv(out->id, GL_QUERY_RESULT, &out->result));
}
