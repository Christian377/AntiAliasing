// SMAA Edge Detection Pass Fragment Shader 
// Detects edges in the input scene based on luminance (Luma) contrast
// Stores edge strength in the RG channels of the output framebuffer
in vec2 vTexCoord;
in vec4 vOffset[3];

out vec4 FragColor;

uniform sampler2D sceneTex;

void main()
{
    FragColor = vec4(SMAALumaEdgeDetectionPS(vTexCoord, vOffset, sceneTex), 0.0, 0.0);
}