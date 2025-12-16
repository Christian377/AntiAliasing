// SMAA Blend Weight Calculation Pass Fragment Shader 
// Uses the edges detected in Pass 1 to calculate blending weights for pixels
// by searching the Area and Search textures
in vec2 vTexCoord;
in vec2 vPixCoord;
in vec4 vOffset[3];

out vec4 FragColor;

uniform sampler2D edgeTex;
uniform sampler2D areaTex;
uniform sampler2D searchTex;

void main()
{
    FragColor = SMAABlendingWeightCalculationPS(vTexCoord, vPixCoord, vOffset, edgeTex, areaTex, searchTex, vec4(0.0));
}