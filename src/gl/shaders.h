#ifndef __HG_AA_GL_SHADERS
#define __HG_AA_GL_SHADERS

/// @brief Loads a file to memory
/// @param file_path The file path
/// @return Content of the file (must be freed using `free`) or NULL on errors
char* aa_load_file(const char* file_path);

#endif // !__HG_AA_GL_SHADERS
