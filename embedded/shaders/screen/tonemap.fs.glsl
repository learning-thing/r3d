/*
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#version 330 core

/* === Definitions === */

#define TONEMAP_LINEAR 0
#define TONEMAP_REINHARD 1
#define TONEMAP_FILMIC 2
#define TONEMAP_ACES 3

/* === Varyings === */

in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uTexColor;
uniform lowp int uTonemapMode;
uniform float uTonemapExposure;
uniform float uTonemapWhite;

/* === Fragments === */

out vec4 FragColor;

// === Helper functions === //

// Based on Reinhard's extended formula, see equation 4 in https://doi.org/cjbgrt
vec3 TonemapReinhard(vec3 color, float pWhite)
{
    float whiteSquared = pWhite * pWhite;
    vec3 whiteSquaredColor = whiteSquared * color;
    // Equivalent to color * (1 + color / whiteSquared) / (1 + color)
    return (whiteSquaredColor + color * color) / (whiteSquaredColor + whiteSquared);
}

vec3 TonemapFilmic(vec3 color, float pWhite)
{
    // exposure bias: input scale (color *= bias, white *= bias) to make the brightness consistent with other tonemappers
    // also useful to scale the input to the range that the uTonemapMode is designed for (some require very high input values)
    // has no effect on the curve's general shape or visual properties
    const float exposureBias = 2.0f;
    const float A = 0.22f * exposureBias * exposureBias; // bias baked into constants for performance
    const float B = 0.30f * exposureBias;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.01f;
    const float F = 0.30f;

    vec3 colorTonemapped = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float pWhiteTonemapped = ((pWhite * (A * pWhite + C * B) + D * E) / (pWhite * (A * pWhite + B) + D * F)) - E / F;

    return colorTonemapped / pWhiteTonemapped;
}

// Adapted from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// (MIT License).
vec3 TonemapACES(vec3 color, float pWhite)
{
    const float exposureBias = 1.8f;
    const float A = 0.0245786f;
    const float B = 0.000090537f;
    const float C = 0.983729f;
    const float D = 0.432951f;
    const float E = 0.238081f;

    // Exposure bias baked into transform to save shader instructions. Equivalent to `color *= exposureBias`
    const mat3 rgb_to_rrt = mat3(
        vec3(0.59719f * exposureBias, 0.35458f * exposureBias, 0.04823f * exposureBias),
        vec3(0.07600f * exposureBias, 0.90834f * exposureBias, 0.01566f * exposureBias),
        vec3(0.02840f * exposureBias, 0.13383f * exposureBias, 0.83777f * exposureBias)
    );

    const mat3 odt_to_rgb = mat3(
        vec3(1.60475f, -0.53108f, -0.07367f),
        vec3(-0.10208f, 1.10813f, -0.00605f),
        vec3(-0.00327f, -0.07276f, 1.07602f)
    );

    color *= rgb_to_rrt;
    vec3 colorTonemapped = (color * (color + A) - B) / (color * (C * color + D) + E);
    colorTonemapped *= odt_to_rgb;

    pWhite *= exposureBias;
    float pWhiteTonemapped = (pWhite * (pWhite + A) - B) / (pWhite * (C * pWhite + D) + E);

    return colorTonemapped / pWhiteTonemapped;
}

vec3 Tonemapping(vec3 color, float pWhite) // inputs are LINEAR
{
    // Ensure color values passed to tonemappers are positive.
    // They can be negative in the case of negative lights, which leads to undesired behavior.
    if (uTonemapMode == TONEMAP_REINHARD) return TonemapReinhard(max(vec3(0.0f), color), pWhite);
    if (uTonemapMode == TONEMAP_FILMIC) return TonemapFilmic(max(vec3(0.0f), color), pWhite);
    if (uTonemapMode == TONEMAP_ACES) return TonemapACES(max(vec3(0.0f), color), pWhite);
    return color; // TONEMAP_LINEAR
}


// === Main program === //

void main()
{
    // Sampling scene color texture
    vec3 result = texture(uTexColor, vTexCoord).rgb;

    // Appply tonemapping
    result *= uTonemapExposure;
    result = Tonemapping(result, uTonemapWhite);

    // Final color output
    FragColor = vec4(result, 1.0);
}
