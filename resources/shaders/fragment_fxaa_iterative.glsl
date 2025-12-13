// #version 430 core

// in vec2 frag_uv;
// out vec4 FragColor;

// uniform sampler2D screenTexture;
// uniform vec2 resolution;

// // FXAA Quality Settings (close to FXAA 3.11 high quality behavior)
// #define FXAA_QUALITY_SUBPIX              0.75
// #define FXAA_QUALITY_EDGE_THRESHOLD      0.125
// #define FXAA_QUALITY_EDGE_THRESHOLD_MIN  0.0625

// void main()
// {
//     vec2 rcpFrame = 1.0 / resolution;
//     vec2 posM = frag_uv;

//     vec4 rgbyM = texture(screenTexture, posM);
//     float lumaCenter = dot(rgbyM.rgb, vec3(0.299, 0.587, 0.114));

//     // Luma at neighbors
//     float lumaS = dot(textureOffset(screenTexture, posM, ivec2( 0, 1)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaE = dot(textureOffset(screenTexture, posM, ivec2( 1, 0)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaN = dot(textureOffset(screenTexture, posM, ivec2( 0,-1)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaW = dot(textureOffset(screenTexture, posM, ivec2(-1, 0)).rgb, vec3(0.299, 0.587, 0.114));

//     // Contrast check (Early Exit)
//     float maxSM = max(lumaS, lumaCenter);
//     float minSM = min(lumaS, lumaCenter);
//     float maxESM = max(lumaE, maxSM);
//     float minESM = min(lumaE, minSM);
//     float maxWN  = max(lumaN, lumaW);
//     float minWN  = min(lumaN, lumaW);

//     float rangeMax = max(maxWN, maxESM);
//     float rangeMin = min(minWN, minESM);
//     float range    = rangeMax - rangeMin;

//     float rangeMaxScaled  = rangeMax * FXAA_QUALITY_EDGE_THRESHOLD;
//     float rangeMaxClamped = max(FXAA_QUALITY_EDGE_THRESHOLD_MIN, rangeMaxScaled);

//     if (range < rangeMaxClamped) {
//         FragColor = rgbyM;
//         return;
//     }

//     // EDGE DETECTION 
//     float lumaNW = dot(textureOffset(screenTexture, posM, ivec2(-1,-1)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaSE = dot(textureOffset(screenTexture, posM, ivec2( 1, 1)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaNE = dot(textureOffset(screenTexture, posM, ivec2( 1,-1)).rgb, vec3(0.299, 0.587, 0.114));
//     float lumaSW = dot(textureOffset(screenTexture, posM, ivec2(-1, 1)).rgb, vec3(0.299, 0.587, 0.114));

//     float lumaNS = lumaN + lumaS;
//     float lumaWE = lumaW + lumaE;

//     float subpixRcpRange = 1.0 / max(range, 1e-6);
//     float subpixNSWE     = lumaNS + lumaWE;

//     float edgeHorz1 = (-2.0 * lumaCenter) + lumaNS;
//     float edgeVert1 = (-2.0 * lumaCenter) + lumaWE;

//     float lumaNESE = lumaNE + lumaSE;
//     float lumaNWNE = lumaNW + lumaNE;
//     float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
//     float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

//     float lumaNWSW = lumaNW + lumaSW;
//     float lumaSWSE = lumaSW + lumaSE;
//     float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
//     float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;

//     float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
//     float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
//     float edgeHorz  = abs(edgeHorz3) + edgeHorz4;
//     float edgeVert  = abs(edgeVert3) + edgeVert4;

//     float subpixNWSWNESE = lumaNWSW + lumaNESE;

//     bool horzSpan = edgeHorz >= edgeVert;
//     float lengthSign = rcpFrame.x;
//     if (horzSpan) lengthSign = rcpFrame.y;

//     // pick pair along the span direction
//     if (!horzSpan) { lumaN = lumaW; lumaS = lumaE; }

//     float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
//     float subpixB = (subpixA * (1.0/12.0)) - lumaCenter;

//     float gradientN = lumaN - lumaCenter;
//     float gradientS = lumaS - lumaCenter;
//     float lumaNN = lumaN + lumaCenter;
//     float lumaSS = lumaS + lumaCenter;

//     bool  pairN   = abs(gradientN) >= abs(gradientS);
//     float gradient = max(abs(gradientN), abs(gradientS));
//     if (pairN) lengthSign = -lengthSign;

//     float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

//     // base position half-step toward the chosen side
//     vec2 posB = posM;
//     vec2 offNP = vec2(0.0);
//     offNP.x = (!horzSpan) ? rcpFrame.x : 0.0;
//     offNP.y = ( horzSpan) ? rcpFrame.y : 0.0;

//     if (!horzSpan) posB.x += lengthSign * 0.5;
//     else           posB.y += lengthSign * 0.5;

//     vec2 posN = posB - offNP * 1.5;
//     vec2 posP = posB + offNP * 1.5;

//     // subpixel shaping (kept stable, not overly aggressive)
//     float subpixD = (-2.0 * subpixC) + 3.0;
//     float subpixE = subpixC * subpixC;
//     float subpixF = subpixD * subpixE;

//     if (!pairN) lumaNN = lumaSS;

//     float gradientScaled = gradient * (1.0/4.0);
//     float lumaMM = lumaCenter - lumaNN * 0.5;
//     bool  lumaMLTZero = (lumaMM < 0.0);

//     float lumaEndN = dot(texture(screenTexture, posN).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
//     float lumaEndP = dot(texture(screenTexture, posP).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;

//     bool doneN = abs(lumaEndN) >= gradientScaled;
//     bool doneP = abs(lumaEndP) >= gradientScaled;

//     if (!doneN) posN -= offNP * 1.5;
//     if (!doneP) posP += offNP * 1.5;

//     const float steps[12] = float[12](
//         1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0
//     );

//     for (int i = 0; i < 12; i++) {
//         if (doneN && doneP) break;

//         float stepSize = steps[i];

//         if (!doneN) {
//             lumaEndN = dot(texture(screenTexture, posN).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
//             doneN = abs(lumaEndN) >= gradientScaled;
//             if (!doneN) posN -= offNP * stepSize;
//         }

//         if (!doneP) {
//             lumaEndP = dot(texture(screenTexture, posP).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
//             doneP = abs(lumaEndP) >= gradientScaled;
//             if (!doneP) posP += offNP * stepSize;
//         }
//     }

//     // distance along the march axis (IMPORTANT: use Y when horzSpan, X when vertSpan)
//     float dstN = horzSpan ? (posM.y - posN.y) : (posM.x - posN.x);
//     float dstP = horzSpan ? (posP.y - posM.y) : (posP.x - posM.x);
//     dstN = abs(dstN);
//     dstP = abs(dstP);

//     float spanLength = dstN + dstP;
//     float spanLengthRcp = 1.0 / max(spanLength, 1e-6);

//     bool directionN = dstN < dstP;
//     float dst = min(dstN, dstP);

//     bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
//     bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
//     bool goodSpan  = directionN ? goodSpanN : goodSpanP;

//     float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
//     float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;

//     float subpixH = clamp(subpixF * FXAA_QUALITY_SUBPIX, 0.0, 0.75);
//     float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);

//     if (!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
//     else           posM.y += pixelOffsetSubpix * lengthSign;

//     FragColor = vec4(texture(screenTexture, posM).rgb, 1.0);
// }
#version 430 core

in vec2 frag_uv;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;

// FXAA Quality Settings
#define FXAA_QUALITY_SUBPIX              0.75
#define FXAA_QUALITY_EDGE_THRESHOLD      0.125
#define FXAA_QUALITY_EDGE_THRESHOLD_MIN  0.0625

void main()
{
    vec2 rcpFrame = 1.0 / resolution;
    vec2 posM = frag_uv;

    vec4 rgbyM = texture(screenTexture, posM);
    float lumaCenter = dot(rgbyM.rgb, vec3(0.299, 0.587, 0.114));

    // Luma at neighbors
    float lumaS = dot(textureOffset(screenTexture, posM, ivec2( 0, 1)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaE = dot(textureOffset(screenTexture, posM, ivec2( 1, 0)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaN = dot(textureOffset(screenTexture, posM, ivec2( 0,-1)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaW = dot(textureOffset(screenTexture, posM, ivec2(-1, 0)).rgb, vec3(0.299, 0.587, 0.114));

    // Contrast check (Early Exit)
    float maxSM = max(lumaS, lumaCenter);
    float minSM = min(lumaS, lumaCenter);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN  = max(lumaN, lumaW);
    float minWN  = min(lumaN, lumaW);

    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float range    = rangeMax - rangeMin;

    float rangeMaxScaled  = rangeMax * FXAA_QUALITY_EDGE_THRESHOLD;
    float rangeMaxClamped = max(FXAA_QUALITY_EDGE_THRESHOLD_MIN, rangeMaxScaled);

    if (range < rangeMaxClamped) {
        FragColor = rgbyM;
        return;
    }

    // --- EDGE DETECTION ---
    float lumaNW = dot(textureOffset(screenTexture, posM, ivec2(-1,-1)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaSE = dot(textureOffset(screenTexture, posM, ivec2( 1, 1)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaNE = dot(textureOffset(screenTexture, posM, ivec2( 1,-1)).rgb, vec3(0.299, 0.587, 0.114));
    float lumaSW = dot(textureOffset(screenTexture, posM, ivec2(-1, 1)).rgb, vec3(0.299, 0.587, 0.114));

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;

    float subpixRcpRange = 1.0 / max(range, 1e-6);
    float subpixNSWE     = lumaNS + lumaWE;

    float edgeHorz1 = (-2.0 * lumaCenter) + lumaNS;
    float edgeVert1 = (-2.0 * lumaCenter) + lumaWE;

    float lumaNESE  = lumaNE + lumaSE;
    float lumaNWNE  = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    float lumaNWSW  = lumaNW + lumaSW;
    float lumaSWSE  = lumaSW + lumaSE;
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;

    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz  = abs(edgeHorz3) + edgeHorz4;
    float edgeVert  = abs(edgeVert3) + edgeVert4;

    float subpixNWSWNESE = lumaNWSW + lumaNESE;

    // If edge is more horizontal, march in X. If more vertical, march in Y.
    bool horzSpan = edgeHorz >= edgeVert;

    // Signed step size perpendicular to the edge (for blending direction)
    float lengthSign = horzSpan ? rcpFrame.y : rcpFrame.x;

    // pick pair along the span direction (so gradients are along the march axis)
    if (!horzSpan) { lumaN = lumaW; lumaS = lumaE; }

    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
    float subpixB = (subpixA * (1.0/12.0)) - lumaCenter;

    float gradientN = lumaN - lumaCenter;
    float gradientS = lumaS - lumaCenter;
    float lumaNN = lumaN + lumaCenter;
    float lumaSS = lumaS + lumaCenter;

    bool  pairN    = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if (pairN) lengthSign = -lengthSign;

    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

    // base position half-step toward the chosen side
    vec2 posB = posM;

    // march axis step (ALONG the edge)
    vec2 offNP = vec2(0.0);
    offNP.x = ( horzSpan) ? rcpFrame.x : 0.0; // Horizontal edge -> march X
    offNP.y = (!horzSpan) ? rcpFrame.y : 0.0; // Vertical edge   -> march Y

    if (!horzSpan) posB.x += lengthSign * 0.5;
    else           posB.y += lengthSign * 0.5;

    vec2 posN = posB - offNP * 1.5;
    vec2 posP = posB + offNP * 1.5;

    // subpixel shaping
    float subpixD = (-2.0 * subpixC) + 3.0;
    float subpixE = subpixC * subpixC;
    float subpixF = subpixD * subpixE;

    if (!pairN) lumaNN = lumaSS;

    float gradientScaled = gradient * 0.25;
    float lumaMM = lumaCenter - lumaNN * 0.5;
    bool  lumaMLTZero = (lumaMM < 0.0);

    float lumaEndN = dot(texture(screenTexture, posN).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
    float lumaEndP = dot(texture(screenTexture, posP).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;

    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;

    // Optional extra step
    if (!doneN) posN -= offNP * 1.5;
    if (!doneP) posP += offNP * 1.5;

    // Search steps
    const float steps[12] = float[12](
        1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0
    );

    for (int i = 0; i < 12; i++) {
        if (doneN && doneP) break;

        float stepSize = steps[i];

        if (!doneN) {
            lumaEndN = dot(texture(screenTexture, posN).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            if (!doneN) posN -= offNP * stepSize;
        }

        if (!doneP) {
            lumaEndP = dot(texture(screenTexture, posP).rgb, vec3(0.299,0.587,0.114)) - lumaNN * 0.5;
            doneP = abs(lumaEndP) >= gradientScaled;
            if (!doneP) posP += offNP * stepSize;
        }
    }

    // distance along the march axis (X when horzSpan, Y when !horzSpan)
    float dstN = horzSpan ? (posM.x - posN.x) : (posM.y - posN.y);
    float dstP = horzSpan ? (posP.x - posM.x) : (posP.y - posM.y);
    dstN = abs(dstN);
    dstP = abs(dstP);

    float spanLength = dstN + dstP;
    float spanLengthRcp = 1.0 / max(spanLength, 1e-6);

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    bool goodSpan  = directionN ? goodSpanN : goodSpanP;

    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;

    float subpixH = clamp(subpixF * FXAA_QUALITY_SUBPIX, 0.0, 0.75);
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);

    // apply final offset along the march axis
    if (!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    else           posM.y += pixelOffsetSubpix * lengthSign;

    FragColor = vec4(texture(screenTexture, posM).rgb, 1.0);
}