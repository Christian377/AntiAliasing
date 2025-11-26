#include "shaders.h"

const char* VERTEX_DEFAULT = "#version 430 core\n"
                             "layout (location = 0) in vec3 pos;\n"
                             "void main()\n"
                             "{\n"
                             "  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
                             "}\0";

const char* FRAGMENT_DEFAULT = "#version 430 core\n"
                               "out vec4 FragColor;\n"
                               "void main()\n"
                               "{\n"
                               "  FragColor =  vec4(1.0, 1.0, 1.0, 1.0);"
                               "}\0";
