#version 430 core

in vec2 frag_uv;
out vec4 FragColor;

uniform sampler2D sceneTex; 
uniform sampler2D blendTex; 

uniform vec2 resolution; // screen resolution in pixels
void main()
{
vec2 texelSize = 1.0 / resolution;

vec4 color = texture(sceneTex, frag_uv);
vec2 blend = texture(blendTex, frag_uv).rg;

// Compute blended color using neighborhood offsets
vec4 colorLeft  = texture(sceneTex, frag_uv - vec2(texelSize.x, 0.0));
vec4 colorRight = texture(sceneTex, frag_uv + vec2(texelSize.x, 0.0));
vec4 colorUp    = texture(sceneTex, frag_uv - vec2(0.0, texelSize.y));
vec4 colorDown  = texture(sceneTex, frag_uv + vec2(0.0, texelSize.y));

// Mix horizontally and vertically based on blend weights
color.rgb = mix(color.rgb, colorLeft.rgb, blend.r);
color.rgb = mix(color.rgb, colorRight.rgb, blend.r);
color.rgb = mix(color.rgb, colorUp.rgb, blend.g);
color.rgb = mix(color.rgb, colorDown.rgb, blend.g);

FragColor = color;

}