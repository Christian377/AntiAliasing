layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 vTexCoord;
out vec4 vOffset;

void main()
{
    vTexCoord = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
    
    // Calculate offsets for neighborhood blending
    SMAANeighborhoodBlendingVS(vTexCoord, vOffset);
}