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

/* === Defines === */

#define PI 3.1415926535897932384626433832795028

#define DIRLIGHT    0
#define SPOTLIGHT   1
#define OMNILIGHT   2

/* === Structs === */

struct Light
{
    mat4 matViewProj;
    sampler2D shadowMap;
    samplerCube shadowCubemap;
    vec3 color;
    vec3 position;
    vec3 direction;
    float specular;
    float energy;
    float range;
    float attenuation;
    float innerCutOff;
    float outerCutOff;
    float shadowMapTxlSz;
    float shadowBias;
    lowp int type;
    bool shadow;
};

/* === Varyings === */

noperspective in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uTexAlbedo;
uniform sampler2D uTexNormal;
uniform sampler2D uTexDepth;
uniform sampler2D uTexORM;

uniform Light uLight;

uniform vec3 uViewPosition;
uniform mat4 uMatInvProj;
uniform mat4 uMatInvView;

/* === Fragments === */

layout(location = 0) out vec4 FragDiffuse;
layout(location = 1) out vec4 FragSpecular;

/* === PBR functions === */

float DistributionGGX(float cosTheta, float alpha)
{
    // Standard GGX/Trowbridge-Reitz distribution - optimized form
    float a = cosTheta * alpha;
    float k = alpha / (1.0 - cosTheta * cosTheta + a * a);
    return k * k * (1.0 / PI);
}

float GeometryGGX(float NdotL, float NdotV, float roughness)
{
    // Hammon's optimized approximation for GGX Smith geometry term
    // This version is an efficient approximation that:
    // 1. Avoids expensive square root calculations
    // 2. Combines both G1 terms into a single expression
    // 3. Provides very close results to the exact version at a much lower cost
    // SEE: https://www.gdcvault.com/play/1024478/PBR-Diffuse-Lighting-for-GGX
    return 0.5 / mix(2.0 * NdotL * NdotV, NdotL + NdotV, roughness);
}

float SchlickFresnel(float u)
{
    float m = 1.0 - u;
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

vec3 ComputeF0(float metallic, float specular, vec3 albedo)
{
    float dielectric = 0.16 * specular * specular;
    // use (albedo * metallic) as colored specular reflectance at 0 angle for metallic materials
    // SEE: https://google.github.io/filament/Filament.md.html
    return mix(vec3(dielectric), albedo, vec3(metallic));
}

/* === Shadow functions === */

float ShadowOmni(vec3 position, float cNdotL)
{
    // Calculate the direction vector from the light to the fragment
    vec3 lightToFrag = position - uLight.position;
    
    // Calculate the current depth, which is the distance from the light to the fragment
    float currentDepth = length(lightToFrag);
    
    // Compute the bias to reduce shadow acne, considering the angle of the surface normal
    float bias = uLight.shadowBias * max(1.0 - cNdotL, 0.05);

    // Normalize the light-to-fragment vector to get the sampling direction
    vec3 sampleDir = normalize(lightToFrag);
    
    // Generate an orthonormal basis for the tangent and bitangent
    // The 'up' vector is used to generate the tangent and bitangent
    vec3 up = vec3(0, 1, 0);
    
    // If the sample direction is almost parallel to 'up', choose a different 'up' vector
    if (abs(dot(sampleDir, up)) > 0.99) 
        up = vec3(1, 0, 0);
    
    // Calculate the tangent and bitangent vectors, which are perpendicular to the sample direction
    vec3 tangent = normalize(cross(sampleDir, up));
    vec3 bitangent = cross(tangent, sampleDir);

    float shadow = 0.0;
    
    // Define the number of samples for PCF and the sampling radius
    const float SAMPLES = 25.0;     // Set the number of samples for the PCF filter
    const float radius = 0.01;      // Sampling radius (could be made a uniform if needed)
    const int KERNEL_SIZE = 2;      // Kernel size (controls the area sampled around each point)

    // Loop through the kernel area to sample neighboring shadow texels
    for(int x = -KERNEL_SIZE; x <= KERNEL_SIZE; x++) {
        float xOffset = float(x) * radius;
        
        for(int y = -KERNEL_SIZE; y <= KERNEL_SIZE; y++) {
            float yOffset = float(y) * radius;

            // Calculate the offset direction using the tangent and bitangent vectors
            vec3 offset = tangent * xOffset + bitangent * yOffset;
            
            // Calculate the sample vector by adding the offset to the light-to-fragment direction
            vec3 sampleVec = lightToFrag + offset;
            
            // Retrieve the closest depth value from the shadow cubemap for the sample position
            float closestDepth = texture(uLight.shadowCubemap, sampleVec).r;
            
            // Compare the fragment's depth to the closest depth in the shadow map
            // The step function returns 1.0 if the fragment is in shadow, 0.0 otherwise
            shadow += step(closestDepth, currentDepth - bias);
        }
    }
    
    // Return the final shadow factor, averaged over the number of samples
    return 1.0 - (shadow / SAMPLES);
}

float Shadow(vec3 position, float cNdotL)
{
    // Transform the world-space position into light-space using the shadow projection matrix
    vec4 p = uLight.matViewProj * vec4(position, 1.0);

    // Convert from homogeneous clip space to normalized device coordinates (NDC)
    vec3 projCoords = p.xyz / p.w;
    
    // Transform NDC coordinates from [-1,1] range to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if the fragment is outside the shadow map bounds
    // If it is, return 1.0 (fully lit)
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0;

    // Compute the bias to reduce shadow acne (self-shadowing artifacts)
    float bias = max(uLight.shadowBias * (1.0 - cNdotL), 0.00002) + 0.00001;
    
    // Apply the bias to the depth value
    projCoords.z -= bias;

    float shadow = 0.0;

    // Define PCF (Percentage Closer Filtering) parameters
    const float SAMPLES = 25.0;     // Number of samples for PCF
    const float radius = 0.0005;    // Sampling radius (can be made a uniform for dynamic control)
    const int KERNEL_SIZE = 2;      // Kernel size for sampling around the main shadow coordinate

    // Generate an orthonormal basis for sampling around the main shadow coordinate
    vec3 up = vec3(0, 1, 0);
    
    // Avoid degenerate cases where 'up' is parallel to the sampling direction
    if (abs(dot(vec3(0, 0, 1), up)) > 0.99) 
        up = vec3(1, 0, 0);
    
    // Compute the tangent and bitangent vectors
    vec3 tangent = normalize(cross(vec3(0, 0, 1), up));
    vec3 bitangent = cross(tangent, vec3(0, 0, 1));

    // PCF Loop: Sample surrounding texels in the shadow map
    for (int x = -KERNEL_SIZE; x <= KERNEL_SIZE; x++)
    {
        float xOffset = float(x) * radius;
        for (int y = -KERNEL_SIZE; y <= KERNEL_SIZE; y++)
        {
            float yOffset = float(y) * radius;

            // Compute sampling offset using the tangent and bitangent vectors
            vec2 offset = tangent.xy * xOffset + bitangent.xy * yOffset;
            
            // Retrieve the closest depth value from the shadow map
            float closestDepth = texture(uLight.shadowMap, projCoords.xy + offset).r;
            
            // Compare depth values: if the current fragment is in shadow, add to shadow factor
            shadow += step(projCoords.z, closestDepth);
        }
    }

    // Return the final shadow factor (normalized over the number of samples)
    return shadow / SAMPLES;
}


/* === Misc functions === */

vec3 GetPositionFromDepth(float depth)
{
    vec4 ndcPos = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = uMatInvProj * ndcPos;
    viewPos /= viewPos.w;

    return (uMatInvView * viewPos).xyz;
}

vec3 DecodeOctahedral(vec2 encoded)
{
    // Remap from [0,1] to [-1,1]
    vec2 f = encoded * 2.0 - 1.0;
    
    // Initial reconstruction
    vec3 normal = vec3(f.xy, 1.0 - abs(f.x) - abs(f.y));
    
    // Unfold if outside the octahedron (also avoids the use of boolean vectors)
    if (normal.z < 0.0) {
        vec2 signValue = vec2(normal.x >= 0.0 ? 1.0 : -1.0, normal.y >= 0.0 ? 1.0 : -1.0);
        normal.xy = (1.0 - abs(normal.yx)) * signValue;
    }
    
    // Final normalization
    return normalize(normal);
}

vec3 RotateWithQuat(vec3 v, vec4 q)
{
    vec3 t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}


/* === Main === */

void main()
{
    /* Sample albedo and ORM texture and extract values */
    
    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;
    vec3 orm = texture(uTexORM, vTexCoord).rgb;
    float roughness = orm.g;
    float metalness = orm.b;

    /* Compute F0 (reflectance at normal incidence) based on the metallic factor */

    vec3 F0 = ComputeF0(metalness, 0.5, albedo);

    /* Sample world depth and reconstruct world position */

    float depth = texture(uTexDepth, vTexCoord).r;
    vec3 position = GetPositionFromDepth(depth);

    /* Sample and decode normal in world space */

    vec3 N = DecodeOctahedral(texture(uTexNormal, vTexCoord).rg);

    /* Compute view direction and the dot product of the normal and view direction */
    
    vec3 V = normalize(uViewPosition - position);

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4); // Clamped to avoid division by zero

    /* Compute light direction and the dot product of the normal and light direction */

    vec3 L = (uLight.type == DIRLIGHT) ? -uLight.direction : normalize(uLight.position - position);

    float NdotL = max(dot(N, L), 0.0);
    float cNdotL = min(NdotL, 1.0); // Clamped to avoid division by zero

    /* Compute the halfway vector between the view and light directions */

    vec3 H = normalize(V + L);

    float LdotH = max(dot(L, H), 0.0);
    float cLdotH = min(dot(L, H), 1.0);

    float NdotH = max(dot(N, H), 0.0);
    float cNdotH = min(NdotH, 1.0);

    /* Compute light color energy */

    vec3 lightColE = uLight.color * uLight.energy;

    /* Compute diffuse lighting */

    vec3 diffuse = vec3(0.0);

    if (metalness < 1.0)
    {
        float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
        float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
        float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);

        float diffBRDF = (1.0 / PI) * (FdV * FdL * cNdotL);
        diffuse = diffBRDF * lightColE;
    }

    /* Compute specular lighting */

    // NOTE: When roughness is 0, specular light should not be entirely disabled.

    vec3 specular = vec3(0.0);

    if (roughness > 0.0)
    {
        float alphaGGX = roughness * roughness;
        float D = DistributionGGX(cNdotH, alphaGGX);
        float G = GeometryGGX(cNdotL, cNdotV, alphaGGX);

        float cLdotH5 = SchlickFresnel(cLdotH);
        float F90 = clamp(50.0 * F0.g, 0.0, 1.0);
        vec3 F = F0 + (F90 - F0) * cLdotH5;

        vec3 specBRDF = cNdotL * D * F * G;
        specular = specBRDF * lightColE * uLight.specular;
    }

    /* Apply shadow factor in addition to the SSAO if the light casts shadows */

    float shadow = 1.0;

    if (uLight.shadow)
    {
        if (uLight.type != OMNILIGHT) shadow = Shadow(position, cNdotL);
        else shadow = ShadowOmni(position, cNdotL);
    }

    /* Apply attenuation based on the distance from the light */

    if (uLight.type != DIRLIGHT)
    {
        float dist = length(uLight.position - position);
        float atten = 1.0 - clamp(dist / uLight.range, 0.0, 1.0);
        shadow *= atten * uLight.attenuation;
    }

    /* Apply spotlight effect if the light is a spotlight */

    if (uLight.type == SPOTLIGHT)
    {
        float theta = dot(L, -uLight.direction);
        float epsilon = (uLight.innerCutOff - uLight.outerCutOff);
        shadow *= smoothstep(0.0, 1.0, (theta - uLight.outerCutOff) / epsilon);
    }

    /* Compute final lighting contribution */
    
    FragDiffuse = vec4(diffuse * shadow, 1.0);
    FragSpecular = vec4(specular * shadow, 1.0);
}
