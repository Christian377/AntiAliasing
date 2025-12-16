// Renders a full-screen quad for post-processing effects
// Passes UV coordinates to the fragment shader for texture sampling
#version 430 core
layout(location = 0) in vec3 pos; 
layout(location = 1) in vec2 uv; 
// UVs will be interpolated automatically to correspond to the exact fragment coordinates on the texture
out vec2 frag_uv; 
void main() 
{ 
    frag_uv = uv; 
    gl_Position = vec4(pos, 1.0);
}