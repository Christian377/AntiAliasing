cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0079 NEW)

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
# Dependencies: ImGUI
#######################
set(IMGUI_STATIC TRUE CACHE STRING "" FORCE)
add_subdirectory("3rd_party/cimgui")
target_sources(cimgui PRIVATE
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_opengl3.cpp"
)
target_compile_definitions(cimgui PRIVATE
    "-DIMGUI_IMPL_API=extern \"C\"" # needed to remove mangling
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)
target_link_libraries(cimgui glfw glad)

#######################
# Output Executable
#######################
file(GLOB_RECURSE AA_FILES_C CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE AA_FILES_H CONFIGURE_DEPENDS "src/*.h")

add_executable(aa ${AA_FILES_C} ${AA_FILES_H})
target_link_libraries(aa PUBLIC cimgui)
target_compile_definitions(aa PUBLIC _CRT_SECURE_NO_WARNINGS)

set(AA_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# let VS use AA as the starting project 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT aa)
set_target_properties(aa PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:aa>"
)

add_custom_command(TARGET aa POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:aa>/resources"
)
add_custom_command(TARGET aa POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${AA_BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "${AA_BIN_DIR}/resources"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:aa>"
        "${AA_BIN_DIR}"
)