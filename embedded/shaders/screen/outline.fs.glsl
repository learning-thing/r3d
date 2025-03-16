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

noperspective in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uTexColor;
uniform sampler2D uTexDepth;
uniform sampler2D uTexNormal;

uniform mat4 uMatInvProj;
uniform mat4 uMatView;

uniform vec2 uTexelSize;
uniform float uNear;
uniform float uFar;

uniform vec4 uColOutline;
uniform float uThickness;           //< Outline thickness
uniform float uEdgeSharpness;        //< Sensitivity of normals
uniform float uDepthThreshold;
uniform float uNormalThreshold;

/* === Fragments === */

out vec3 FragColor;

/* === Helper functions === */

vec3 GetPositionFromDepth(float depth)
{
    vec4 ndcPos = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = uMatInvProj * ndcPos;
    viewPos /= viewPos.w;
    // Changed to keep position in view space
    return viewPos.xyz;
}

vec3 DecodeOctahedral(vec2 encoded)
{
    // Remap from [0,1] to [-1,1]
    vec2 f = encoded * 2.0 - 1.0;
    
    // Initial reconstruction
    vec3 normal = vec3(f.xy, 1.0 - abs(f.x) - abs(f.y));
    
    // Unfold if outside the octahedron
    if (normal.z < 0.0) {
        vec2 signValue = vec2(normal.x >= 0.0 ? 1.0 : -1.0, normal.y >= 0.0 ? 1.0 : -1.0);
        normal.xy = (1.0 - abs(normal.yx)) * signValue;
    }
    
    // Transform normal from world to view space
    return normalize(mat3(uMatView) * normal);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}

/* === Main program === */

void main()
{
    vec3 sceneColor = texture(uTexColor, vTexCoord).rgb;
    float centerDepth = texture(uTexDepth, vTexCoord).r;
    float linearCenterDepth = LinearizeDepth(centerDepth);
    vec3 currentNormal = DecodeOctahedral(texture(uTexNormal, vTexCoord).rg);

    const vec2 offsets[8] = vec2[](
        vec2(-1, -1),
        vec2( 0, -1),
        vec2( 1, -1),
        vec2(-1,  0),
        vec2( 1,  0),
        vec2(-1,  1),
        vec2( 0,  1),
        vec2( 1,  1)
    );

    vec4 outlineMod = vec4(0.0);
    for (int i = 0; i < 8; i++)
    {
        vec2 sampleCoord = vTexCoord + offsets[i] * uThickness * uTexelSize;

        float sampleDepth = texture(uTexDepth, sampleCoord).r;
        float linearSampleDepth = LinearizeDepth(sampleDepth);
        float diffDepth = abs(linearCenterDepth - linearSampleDepth);

        vec3 sampleNormal = DecodeOctahedral(texture(uTexNormal, sampleCoord).rg);
        float diffNormal = 1.0 - dot(currentNormal, sampleNormal);
        
        if (diffDepth > uDepthThreshold || (uEdgeSharpness > 0.0 && (uEdgeSharpness * diffNormal > uNormalThreshold)))
        {
            outlineMod = uColOutline;
            break;
        }
    }

    FragColor = mix(sceneColor, outlineMod.rgb, outlineMod.a);
}
