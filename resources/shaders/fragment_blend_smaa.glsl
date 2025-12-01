#version 430 core

in vec2 frag_uv;
out vec4 FragColor;

uniform sampler2D edgeTex; 
uniform sampler2D areaTex; 
uniform sampler2D searchTex; 

uniform vec2 resolution; // screen resolution in pixels

void main()
{
vec2 texelSize = 1.0 / resolution;

vec2 edge = texture(edgeTex, frag_uv).rg;

// If no edge, skip blending
if (edge.r == 0.0 && edge.g == 0.0)
{
    FragColor = vec4(0.0);
    return;
}

// Horizontal and vertical edge blending weights
float blendH = 0.0;
float blendV = 0.0;

// Sample neighbor edges
float leftEdge   = texture(edgeTex, frag_uv - vec2(texelSize.x, 0.0)).r;
float rightEdge  = texture(edgeTex, frag_uv + vec2(texelSize.x, 0.0)).r;
float upEdge     = texture(edgeTex, frag_uv - vec2(0.0, texelSize.y)).g;
float downEdge   = texture(edgeTex, frag_uv + vec2(0.0, texelSize.y)).g;

// Horizontal weight
if (edge.r > 0.0)
    blendH = (leftEdge + rightEdge) * 0.5;

// Vertical weight
if (edge.g > 0.0)
    blendV = (upEdge + downEdge) * 0.5;

// Sample SMAA area texture for subpixel offsets
vec2 areaSample = texture(areaTex, vec2(blendH, blendV)).rg;

FragColor = vec4(areaSample, 0.0, 1.0);

}