#version 430 core

in vec2 frag_uv;
out vec4 outColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;

// Tunables (defaults chosen to match common FXAA 3.11 "quality" usage)
uniform float fxaaQualitySubpix        = 0.75;    // 0.00..1.00
uniform float fxaaQualityEdgeThreshold = 0.166;   // 0.063..0.333 (lower = more edges, slower)
uniform float fxaaQualityEdgeThresholdMin = 0.0833; // 0.0312..0.0833
uniform float lumaScale = 1.0; // keep 1.0 unless you know you need otherwise

vec4 FxaaTexOff(sampler2D t, vec2 p, vec2 o, vec2 r) { return textureLod(t, p + (o * r), 0.0); }
float FxaaLuma(vec4 rgba) { return lumaScale * dot(rgba.rgb, vec3(0.299, 0.587, 0.114)); }
float FxaaSat(float x) { return clamp(x, 0.0, 1.0); }

// FXAA 3.11 (Quality) pixel shader port (long-search/high quality preset)
vec4 FxaaPixelShader(
    vec2 pos,                 // center-of-pixel UV
    sampler2D tex,             // input color
    vec2 fxaaQualityRcpFrame,  // 1.0/width, 1.0/height
    float qSubpix,
    float qEdgeThreshold,
    float qEdgeThresholdMin
) {
    // "high 39" preset search steps (renamed to avoid reserved '__' identifiers)
    float fxaaQualityP0  = 1.0;
    float fxaaQualityP1  = 1.0;
    float fxaaQualityP2  = 1.0;
    float fxaaQualityP3  = 1.0;
    float fxaaQualityP4  = 1.0;
    float fxaaQualityP5  = 1.5;
    float fxaaQualityP6  = 2.0;
    float fxaaQualityP7  = 2.0;
    float fxaaQualityP8  = 2.0;
    float fxaaQualityP9  = 2.0;
    float fxaaQualityP10 = 4.0;
    float fxaaQualityP11 = 8.0;

    vec2 posM = pos;
    vec4 rgbyM = textureLod(tex, posM, 0.0);
    float lumaM = FxaaLuma(rgbyM);

    float lumaS = FxaaLuma(FxaaTexOff(tex, posM, vec2( 0.0, 1.0), fxaaQualityRcpFrame));
    float lumaE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0, 0.0), fxaaQualityRcpFrame));
    float lumaN = FxaaLuma(FxaaTexOff(tex, posM, vec2( 0.0,-1.0), fxaaQualityRcpFrame));
    float lumaW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0, 0.0), fxaaQualityRcpFrame));

    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);
    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float range = rangeMax - rangeMin;

    float rangeMaxScaled = rangeMax * qEdgeThreshold;
    float rangeMaxClamped = max(qEdgeThresholdMin, rangeMaxScaled);

    // Early exit
    if (range < rangeMaxClamped) {
        return rgbyM;
    }

    float lumaNW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0,-1.0), fxaaQualityRcpFrame));
    float lumaSE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0, 1.0), fxaaQualityRcpFrame));
    float lumaNE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0,-1.0), fxaaQualityRcpFrame));
    float lumaSW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0, 1.0), fxaaQualityRcpFrame));

    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;

    float subpixRcpRange = 1.0 / range;
    float subpixNSWE = lumaNS + lumaWE;

    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;

    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;

    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);

    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;

    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;

    float subpixNWSWNESE = lumaNWSW + lumaNESE;

    float lengthSign = fxaaQualityRcpFrame.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;

    if (!horzSpan) lumaN = lumaW;
    if (!horzSpan) lumaS = lumaE;
    if (horzSpan) lengthSign = fxaaQualityRcpFrame.y;

    float subpixB = (subpixA * (1.0 / 12.0)) - lumaM;
    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;

    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if (pairN) lengthSign = -lengthSign;

    float subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);

    vec2 posB = posM;
    vec2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;

    if (!horzSpan) posB.x += lengthSign * 0.5;
    if ( horzSpan) posB.y += lengthSign * 0.5;

    vec2 posN = posB - offNP * fxaaQualityP0;
    vec2 posP = posB + offNP * fxaaQualityP0;

    float subpixD = ((-2.0) * subpixC) + 3.0;
    float lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0));
    float subpixE = subpixC * subpixC;
    float lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0));

    if (!pairN) lumaNN = lumaSS;

    float gradientScaled = gradient * (1.0 / 4.0);
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = subpixD * subpixE;
    bool lumaMLTZero = lumaMM < 0.0;

    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;

    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;

    if (!doneN) posN -= offNP * fxaaQualityP1;
    if (!doneP) posP += offNP * fxaaQualityP1;

    bool doneNP = (!doneN) || (!doneP);

    // Unrolled search (P2..P11)
    if (doneNP) {
        if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP2; }
        if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP2; }
        doneNP = (!doneN) || (!doneP);

        if (doneNP) {
            if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP3; }
            if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP3; }
            doneNP = (!doneN) || (!doneP);

            if (doneNP) {
                if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP4; }
                if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP4; }
                doneNP = (!doneN) || (!doneP);

                if (doneNP) {
                    if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP5; }
                    if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP5; }
                    doneNP = (!doneN) || (!doneP);

                    if (doneNP) {
                        if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP6; }
                        if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP6; }
                        doneNP = (!doneN) || (!doneP);

                        if (doneNP) {
                            if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP7; }
                            if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP7; }
                            doneNP = (!doneN) || (!doneP);

                            if (doneNP) {
                                if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP8; }
                                if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP8; }
                                doneNP = (!doneN) || (!doneP);

                                if (doneNP) {
                                    if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP9; }
                                    if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP9; }
                                    doneNP = (!doneN) || (!doneP);

                                    if (doneNP) {
                                        if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP10; }
                                        if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP10; }
                                        doneNP = (!doneN) || (!doneP);

                                        if (doneNP) {
                                            if (!doneN) { lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0)) - lumaNN * 0.5; doneN = abs(lumaEndN) >= gradientScaled; if (!doneN) posN -= offNP * fxaaQualityP11; }
                                            if (!doneP) { lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0)) - lumaNN * 0.5; doneP = abs(lumaEndP) >= gradientScaled; if (!doneP) posP += offNP * fxaaQualityP11; }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if (!horzSpan) dstN = posM.y - posN.y;
    if (!horzSpan) dstP = posP.y - posM.y;

    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;

    float spanLength = (dstP + dstN);
    float spanLengthRcp = 1.0 / spanLength;

    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;

    float subpixG = subpixF * subpixF;
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float subpixH = subpixG * qSubpix;

    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);

    if (!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if ( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;

    return vec4(textureLod(tex, posM, 0.0).rgb, lumaM);
}

void main() {
    vec2 rcpFrame = 1.0 / resolution;

    vec4 aa = FxaaPixelShader(
        frag_uv,
        screenTexture,
        rcpFrame,
        fxaaQualitySubpix,
        fxaaQualityEdgeThreshold,
        fxaaQualityEdgeThresholdMin
    );

    outColor = vec4(aa.rgb, 1.0);
}
