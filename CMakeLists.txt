cmake_minimum_required(VERSION 3.12)

project(AntiAliasing VERSION 0.0.0.0 LANGUAGES C)

# we only want one single executable at the end
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)

#######################
# Dependencies: GLFW
#######################
add_subdirectory("3rd_party/glfw")

#######################
# Dependencies: GLAD
#######################
add_library(glad STATIC "3rd_party/glad/src/glad.c")
target_include_directories(glad PUBLIC "3rd_party/glad/include")

#######################
# Output Executable
#######################
file(GLOB_RECURSE AA_FILES_C CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE AA_FILES_H CONFIGURE_DEPENDS "src/*.h")

add_executable(aa ${AA_FILES_C} ${AA_FILES_H})
target_link_libraries(aa PUBLIC glfw glad)

# let VS use AA as the starting project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT aa)