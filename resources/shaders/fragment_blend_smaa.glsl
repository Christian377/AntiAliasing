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