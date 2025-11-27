#include "shaders.h"
#include <stdio.h>
#include <stdlib.h>

char* aa_load_file(const char* file_path)
{
  if (file_path == NULL)
    return NULL;

  FILE* file = fopen(file_path, "r");
  if (file == NULL)
    return NULL;

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  if (size < 0)
  {
    fclose(file);
    return NULL;
  }
  fseek(file, 0, SEEK_SET);

  char* ret = malloc(size + 1);
  if (ret == NULL)
  {
    fclose(file);
    return NULL;
  }
  ret[fread(ret, 1, size, file)] = '\0';
  fclose(file);
  return ret;
}