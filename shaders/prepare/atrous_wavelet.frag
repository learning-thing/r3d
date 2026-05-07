/* atrous_wavelet.frag -- À-Trous Wavelet denoiser (depth + normal aware)
 *
 * Copyright (c) 2025-2026 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#version 330 core

/* === Includes === */

#include "../include/math.glsl"

/* === Varyings === */

noperspective in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uSourceTex;
uniform sampler2D uNormalTex;
uniform sampler2D uDepthTex;

uniform float uInvNormalSharp;
uniform float uInvDepthSharp;
uniform float uInvStepWidth2;   // 1.0 / (uStepWidth*uStepWidth)
uniform int uStepWidth;         // Powers of 2: 1, 2, 4, 8... for each pass

/* === Fragments === */

out vec4 FragColor;

/* === Kernel === */

const int KERNEL_SIZE = 8;

const ivec2 OFFSETS[8] = ivec2[8](
    ivec2(-1,-1), ivec2(0,-1), ivec2(1,-1),
    ivec2(-1, 0),              ivec2(1, 0),
    ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1)
);

const float WEIGHTS[8] = float[8](
    0.0625, 0.125, 0.0625,
    0.125,         0.125,
    0.0625, 0.125, 0.0625
);

/* === Helpers === */

#define OFFSCREEN(sc, res) (any(lessThan(sc, ivec2(0))) || any(greaterThanEqual(sc, res)))

/* === Main === */

#ifdef SMART_FILTER

#include "../include/blocks/view.glsl"

void main()
{
    ivec2 resolution = textureSize(uSourceTex, 0);
    ivec2 pixCoord = ivec2(gl_FragCoord.xy);

    vec3 cp = V_GetViewPosition(uDepthTex, pixCoord);
    vec3 cn = V_GetViewNormal(uNormalTex, pixCoord);
    vec4 cc = texelFetch(uSourceTex, pixCoord, 0);

    float invScale2 = 1.0 / dot(cp, cp);
    float viewCos = max(abs(dot(cn, cp)) * sqrt(invScale2), 0.05);
    float angleFactor = min(1.0 / viewCos, 4.0);

    float planeSharp = invScale2 * (angleFactor * angleFactor) * uInvDepthSharp;
    float tangSharp = invScale2 * uInvDepthSharp * 0.1;

    vec4 result = cc * 0.25;
    float weightSum = 0.25;

    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        ivec2 sampleCoord = pixCoord + OFFSETS[i] * uStepWidth;
        if (OFFSCREEN(sampleCoord, resolution)) continue;

        vec3 sp = V_GetViewPosition(uDepthTex, sampleCoord);
        vec3 sn = V_GetViewNormal(uNormalTex, sampleCoord);
        vec4 sc = texelFetch(uSourceTex, sampleCoord, 0);

        vec3 delta = sp - cp;
        float pd = dot(delta, cn);
        float td2 = max(dot(delta, delta) - pd * pd, 0.0);
        float nDiff = dot(cn - sn, cn - sn) * uInvStepWidth2;

        float w = WEIGHTS[i] * exp(
            -nDiff * uInvNormalSharp
            -pd * pd * planeSharp
            -td2 * tangSharp
        );

        result += sc * w;
        weightSum += w;
    }

    FragColor = result / max(weightSum, 1e-4);
}

#else

void main()
{
    ivec2 resolution = textureSize(uSourceTex, 0);
    ivec2 pixCoord = ivec2(gl_FragCoord.xy);

    vec3 cn = M_DecodeOctahedral(texelFetch(uNormalTex, pixCoord, 0).rg);
    float cz = texelFetch(uDepthTex,  pixCoord, 0).r;
    vec4 cc = texelFetch(uSourceTex, pixCoord, 0);

    vec4 result = cc * 0.25;
    float weightSum = 0.25;

    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        ivec2 sampleCoord = pixCoord + OFFSETS[i] * uStepWidth;
        if (OFFSCREEN(sampleCoord, resolution)) continue;

        vec3 sn = M_DecodeOctahedral(texelFetch(uNormalTex, sampleCoord, 0).rg);
        float sz = texelFetch(uDepthTex,  sampleCoord, 0).r;
        vec4 sc = texelFetch(uSourceTex, sampleCoord, 0);

        vec3 dn = cn - sn;
        float dist2 = dot(dn, dn) * uInvStepWidth2;
        float dz = cz - sz;

        float w = WEIGHTS[i] * exp(
            -dist2 * uInvNormalSharp
            -dz*dz * uInvDepthSharp
        );

        result += sc * w;
        weightSum += w;
    }

    FragColor = result / max(weightSum, 1e-4);
}

#endif // SMART_FILTER
