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

/* === Varyings === */

in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uTexture;
uniform vec2 uTexelSize;

uniform float uQualityLevel;     // Between 0.0 (fast) and 1.0 (high quality)
uniform float uEdgeSensitivity;  // Between 0.0 (less sensitive) and 1.0 (more sensitive)
uniform float uSubpixelQuality;  // Between 0.0 (less subpixel AA) and 1.0 (more subpixel AA)

/* === Fragments === */

out vec4 FragColor;

/* === Constants === */

const float EDGE_THRESHOLD_MIN = 0.0312;
const float EDGE_THRESHOLD_MAX = 0.125;
const float ITERATIONS_MAX = 12.0;

/* === Helper functions === */

float rgb2luma(vec3 rgb)
{
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

vec3 sampleOffset(sampler2D tex, vec2 uv, vec2 offset)
{
    return texture(tex, uv + offset).rgb;
}

/* === Main function === */

void main()
{
    // Calculate the number of sampling iterations based on quality setting
    // Higher quality means more iterations for better edge detection and smoother results
    float iterations = mix(4.0, ITERATIONS_MAX, uQualityLevel);
    
    // Adjust edge detection thresholds based on edge sensitivity
    // Lower sensitivity means higher thresholds, reducing the number of edges detected
    float edgeThresholdMin = EDGE_THRESHOLD_MIN * (2.0 - uEdgeSensitivity);
    float edgeThresholdMax = EDGE_THRESHOLD_MAX * (2.0 - uEdgeSensitivity);

    // Sample the center pixel color and calculate its luminance
    vec3 colorCenter = texture(uTexture, vTexCoord).rgb;
    float lumaCenter = rgb2luma(colorCenter);

    // Calculate dynamic offset for sampling neighboring pixels
    // Higher quality reduces offset size for more precise sampling
    float offsetMult = mix(1.0, 0.5, uQualityLevel);
    vec2 offsetSize = uTexelSize * offsetMult;

    // Sample luminance values in cardinal directions (up, down, left, right)
    // These samples form the primary edge detection pattern
    float lumaDown = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(0.0, -offsetSize.y)));
    float lumaUp = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(0.0, offsetSize.y)));
    float lumaLeft = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(-offsetSize.x, 0.0)));
    float lumaRight = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(offsetSize.x, 0.0)));
    
    // Sample luminance values in diagonal directions
    // Additional samples improve edge detection accuracy for diagonal edges
    float lumaDL = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(-offsetSize.x, -offsetSize.y)));
    float lumaUR = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(offsetSize.x, offsetSize.y)));
    float lumaUL = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(-offsetSize.x, offsetSize.y)));
    float lumaDR = rgb2luma(sampleOffset(uTexture, vTexCoord, vec2(offsetSize.x, -offsetSize.y)));

    // Calculate local contrast by finding min/max luminance values
    // This helps identify edges and determine if anti-aliasing is needed
    float lumaMin = min(lumaCenter, min(min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)),
                                      min(min(lumaDL, lumaUR), min(lumaUL, lumaDR))));
    float lumaMax = max(lumaCenter, max(max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)),
                                      max(max(lumaDL, lumaUR), max(lumaUL, lumaDR))));
    float lumaRange = lumaMax - lumaMin;

    // Early exit if contrast is below threshold
    // No anti-aliasing needed for low-contrast areas to preserve performance
    if(lumaRange < max(edgeThresholdMin, lumaMax * edgeThresholdMax)) {
        FragColor = vec4(colorCenter, 1.0);
        return;
    }

    // Calculate edge gradients using a modified Sobel operator
    // This determines the direction of the edge for intelligent sampling
    vec2 dir;
    dir.x = -((lumaLeft + lumaRight) - (2.0 * lumaCenter)) +
            2.0 * ((lumaDL + lumaDR) - (lumaUL + lumaUR));
    dir.y = -((lumaUp + lumaDown) - (2.0 * lumaCenter)) +
            2.0 * ((lumaUL + lumaUR) - (lumaDL + lumaDR));

    // Normalize the gradient direction with safeguard against zero division
    // This ensures stable sampling direction even for very weak edges
    float dirLength = max(abs(dir.x), abs(dir.y)) + 0.0001;
    dir = dir / dirLength;

    // Initialize accumulation variables for progressive sampling
    // This will store the weighted sum of samples along the edge direction
    vec3 rgbAccum = vec3(0.0);
    float weightAccum = 0.0;

    // Perform progressive sampling along the edge direction
    // More iterations = better quality but lower performance
    for(float i = 1.0; i <= iterations; i++)
    {
        // Calculate weight and offset for current iteration
        float weight = 1.0 / (i + 1.0);  // Progressive weight reduction
        float offset = (i / iterations) * mix(0.5, 2.0, uQualityLevel);
        
        // Sample pixels in both directions along the edge
        vec3 rgbN = sampleOffset(uTexture, vTexCoord, -dir * offset * offsetSize).rgb;
        vec3 rgbP = sampleOffset(uTexture, vTexCoord, dir * offset * offsetSize).rgb;
        
        // Accumulate weighted samples
        rgbAccum += (rgbN + rgbP) * weight;
        weightAccum += 2.0 * weight;
    }

    // Calculate final average color from accumulated samples
    vec3 rgbAverage = rgbAccum / weightAccum;

    // Calculate blend factor based on local contrast and subpixel quality setting
    // Higher subpixel quality means stronger anti-aliasing effect
    float blendFactor = clamp(
        sqrt(lumaRange) * mix(0.5, 2.0, uSubpixelQuality),
        0.0,
        1.0
    );

    // Output final color with smooth transition between original and filtered result
    // This preserves detail while reducing aliasing artifacts
    FragColor = vec4(mix(colorCenter, rgbAverage, blendFactor), 1.0);
}
