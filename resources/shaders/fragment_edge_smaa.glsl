#version 430 core

in vec2 frag_uv; 
out vec4 FragColor;

uniform sampler2D sceneTex;
uniform float threshold; 

const vec3 LUMA = vec3(0.299, 0.587, 0.114);

void main()
{
vec2 texelSize = 1.0 / vec2(textureSize(sceneTex, 0));

// Sample 4 neighbors
vec3 center = texture(sceneTex, frag_uv).rgb;
vec3 left   = texture(sceneTex, frag_uv - vec2(texelSize.x, 0.0)).rgb;
vec3 right  = texture(sceneTex, frag_uv + vec2(texelSize.x, 0.0)).rgb;
vec3 up     = texture(sceneTex, frag_uv - vec2(0.0, texelSize.y)).rgb;
vec3 down   = texture(sceneTex, frag_uv + vec2(0.0, texelSize.y)).rgb;

// Convert to luma
float luma_center = dot(center, LUMA);
float luma_left   = dot(left, LUMA);
float luma_right  = dot(right, LUMA);
float luma_up     = dot(up, LUMA);
float luma_down   = dot(down, LUMA);

// Detect horizontal and vertical edges
float edgeH = abs(luma_left - luma_center) + abs(luma_center - luma_right);
float edgeV = abs(luma_up - luma_center) + abs(luma_center - luma_down);

FragColor = vec4(edgeH > threshold ? 1.0 : 0.0,
                 edgeV > threshold ? 1.0 : 0.0,
                 0.0, 1.0);

}