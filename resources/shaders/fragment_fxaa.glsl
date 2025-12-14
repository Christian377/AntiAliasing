#version 430 core

in vec2 frag_uv;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution; // window width and height in pixels

// FXAA settings
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0

// NEW: Add the Threshold setting (same as iterative)
#define FXAA_EDGE_THRESHOLD 0.125
#define FXAA_EDGE_THRESHOLD_MIN 0.0625

void main()
{
    vec2 texel = 1.0 / resolution;
    //Center pixel
    vec3 rgbM = texture(screenTexture, frag_uv).rgb;

    // Sample the luminance of neighboring pixels
    float lumaTL = dot(texture(screenTexture, frag_uv + vec2(-texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaTR = dot(texture(screenTexture, frag_uv + vec2(texel.x, -texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaBL = dot(texture(screenTexture, frag_uv + vec2(-texel.x, texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaBR = dot(texture(screenTexture, frag_uv + vec2(texel.x, texel.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaM  = dot(rgbM, vec3(0.299, 0.587, 0.114));

    // Compute local contrast
    float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
    float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));

    // Calculate range of contrast
    float range = lumaMax - lumaMin;
    
    // If contrast is lower than threshold, it's not an edge.
    // Return original color and skip all math below.
    if(range < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD)) {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    vec2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y =  ((lumaTL + lumaBL) - (lumaTR + lumaBR));

    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * 0.25 * FXAA_REDUCE_MUL, FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * texel;

    // Sample along the edge and blend
    vec3 rgbA = 0.5 * (
        texture(screenTexture, frag_uv + dir * (1.0/3.0 - 0.5)).rgb +
        texture(screenTexture, frag_uv + dir * (2.0/3.0 - 0.5)).rgb
    );
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, frag_uv + dir * -0.5).rgb +
        texture(screenTexture, frag_uv + dir * 0.5).rgb
    );

    float lumaB = dot(rgbB, vec3(0.299, 0.587, 0.114));
    if (lumaB < lumaMin || lumaB > lumaMax)
        FragColor = vec4(rgbA, 1.0);
    else
        FragColor = vec4(rgbB, 1.0);
}
