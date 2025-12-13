in vec2 vTexCoord;
in vec4 vOffset;

out vec4 FragColor;

uniform sampler2D sceneTex;
uniform sampler2D blendTex;

void main()
{
    FragColor = SMAANeighborhoodBlendingPS(vTexCoord, vOffset, sceneTex, blendTex);
}