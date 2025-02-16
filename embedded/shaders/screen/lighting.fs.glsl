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

#define NUM_LIGHTS  8

#define DIRLIGHT    0
#define SPOTLIGHT   1
#define OMNILIGHT   2

/* === Structs === */

struct Light {
    vec3 color;
    vec3 position;
    vec3 direction;
    float energy;
    float range;
    float attenuation;
    float innerCutOff;
    float outerCutOff;
    lowp int type;
    bool enabled;
};

/* === Varyings === */

in vec2 vTexCoord;

/* === Uniforms === */

uniform sampler2D uTexAlbedo;
uniform sampler2D uTexEmission;
uniform sampler2D uTexNormal;
uniform sampler2D uTexDepth;
uniform sampler2D uTexSSAO;
uniform sampler2D uTexORM;
uniform sampler2D uTexID;

uniform vec3 uColAmbient;

uniform samplerCube uCubeIrradiance;
uniform samplerCube uCubePrefilter;
uniform sampler2D uTexBrdfLut;
uniform vec4 uQuatSkybox;
uniform bool uHasSkybox;

uniform Light uLights[NUM_LIGHTS];
uniform float uBloomHdrThreshold;
uniform vec3 uViewPosition;
uniform mat4 uMatInvProj;
uniform mat4 uMatInvView;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragBrightness;

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

float GetBrightness(vec3 color)
{
    return length(color);
}

/* === Main === */

void main()
{
    /* Sample albedo texture */

    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;

    /* Sample material ID */

    float matIdF = texture(uTexID, vTexCoord).r;

    if (matIdF == 0.0) //< If the material is null (background)
    {
        FragBrightness = vec4(vec3(0.0), GetBrightness(albedo));
        FragColor = vec4(albedo, 1.0);
        return;
    }

    /* Sample emission texture */

    vec3 emission = texture(uTexEmission, vTexCoord).rgb;

    /* Sample ORM texture and extract values */

    vec3 orm = texture(uTexORM, vTexCoord).rgb;
    float occlusion = orm.r;
    float roughness = orm.g;
    float metalness = orm.b;

    /* Sample world depth and reconstruct world position */

    float depth = texture(uTexDepth, vTexCoord).r;
    vec3 position = GetPositionFromDepth(depth);

    /* Compute F0 (reflectance at normal incidence) based on the metallic factor */

    vec3 F0 = ComputeF0(metalness, 0.5, albedo);

    /* Sample normal and compute view direction vector */

    vec3 N = DecodeOctahedral(texture(uTexNormal, vTexCoord).rg);
    vec3 V = normalize(uViewPosition - position);

    /* Compute the dot product of the normal and view direction */

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4);  // Clamped to avoid division by zero

    /* Loop through all light sources accumulating diffuse and specular light */

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        if (uLights[i].enabled)
        {
            /* Compute light direction */

            vec3 L = vec3(0.0);
            if (uLights[i].type == DIRLIGHT) L = -uLights[i].direction;
            else L = normalize(uLights[i].position - position);

            /* Compute the dot product of the normal and light direction */

            float NdotL = max(dot(N, L), 0.0);
            float cNdotL = min(NdotL, 1.0); // clamped NdotL

            /* Compute the halfway vector between the view and light directions */

            vec3 H = normalize(V + L);

            float LdotH = max(dot(L, H), 0.0);
            float cLdotH = min(dot(L, H), 1.0);

            float NdotH = max(dot(N, H), 0.0);
            float cNdotH = min(NdotH, 1.0);

            /* Compute light color energy */

            vec3 lightColE = uLights[i].color * uLights[i].energy;

            /* Compute diffuse lighting */

            vec3 diffLight = vec3(0.0);

            if (metalness < 1.0)
            {
                float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
                float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
                float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);

                float diffBRDF = (1.0 / PI) * (FdV * FdL * cNdotL);
                diffLight = diffBRDF * lightColE;
            }

            /* Compute specular lighting */

            vec3 specLight = vec3(0.0);

            // NOTE: When roughness is 0, specular light should not be entirely disabled.
            // TODO: Handle perfect mirror reflection when roughness is 0.

            if (roughness > 0.0)
            {
                float alphaGGX = roughness * roughness;
                float D = DistributionGGX(cNdotH, alphaGGX);
                float G = GeometryGGX(cNdotL, cNdotV, alphaGGX);

                float cLdotH5 = SchlickFresnel(cLdotH);
                float F90 = clamp(50.0 * F0.g, 0.0, 1.0);
                vec3 F = F0 + (F90 - F0) * cLdotH5;

                vec3 specBRDF = cNdotL * D * F * G;
                specLight = specBRDF * lightColE; // (specLight) * uLights[i].specular
            }

            /* Apply shadow factor if the light casts shadows */

            float shadow = 1.0;

            #ifdef RECEIVE_SHADOW
                if (uLights[i].shadow)
                {
                    if (uLights[i].type != OMNILIGHT) shadow = Shadow(i, cNdotL);
                    else shadow = ShadowOmni(i, cNdotL);
                }
            #endif

            /* Apply attenuation based on the distance from the light */

            if (uLights[i].type != DIRLIGHT)
            {
                float dist = length(uLights[i].position - position);
                float atten = 1.0 - clamp(dist / uLights[i].range, 0.0, 1.0);
                shadow *= atten * uLights[i].attenuation;
            }

            /* Apply spotlight effect if the light is a spotlight */

            if (uLights[i].type == SPOTLIGHT)
            {
                float theta = dot(L, -uLights[i].direction);
                float epsilon = (uLights[i].innerCutOff - uLights[i].outerCutOff);
                shadow *= smoothstep(0.0, 1.0, (theta - uLights[i].outerCutOff) / epsilon);
            }

            /* Accumulate the diffuse and specular lighting contributions */

            diffuse += diffLight * shadow;
            specular += specLight * shadow;
        }
    }

    /* Compute ambient - (IBL diffuse) */

    vec3 ambient = uColAmbient;

    if (uHasSkybox)
    {
        vec3 kS = F0 + (1.0 - F0) * SchlickFresnel(cNdotV);
        vec3 kD = (1.0 - kS) * (1.0 - metalness);

        vec3 Nr = RotateWithQuat(N, uQuatSkybox);

        ambient = kD * texture(uCubeIrradiance, Nr).rgb;
    }

    /* Compute ambient occlusion - (from ORM / SSAO) */

    float ssao = texture(uTexSSAO, vTexCoord).r;
    ambient *= occlusion * ssao;

    // Light affect should be material-specific
    //float lightAffect = mix(1.0, ao, uValAOLightAffect);
    //specular *= lightAffect;
    //diffuse *= lightAffect;

    /* Skybox reflection - (IBL specular) */

    if (uHasSkybox)
    {
        vec3 R = RotateWithQuat(reflect(-V, N), uQuatSkybox);

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(uCubePrefilter, R, roughness * MAX_REFLECTION_LOD).rgb;

        float fresnelTerm = SchlickFresnel(cNdotV);
        vec3 F = F0 + (max(vec3(1.0 - roughness), F0) - F0) * fresnelTerm;

        vec2 brdf = texture(uTexBrdfLut, vec2(cNdotV, roughness)).rg;
        vec3 specularReflection = prefilteredColor * (F * brdf.x + brdf.y);

        specular += specularReflection;
    }

    /* Compute the final diffuse color, including ambient and diffuse lighting contributions */

    diffuse = albedo.rgb * (ambient + diffuse);

    /* Compute the final fragment color by combining diffuse, specular, and emission contributions */

    FragColor = vec4(diffuse + specular + emission, 1.0);

    /* Handle bright colors for bloom */

    float brightness = GetBrightness(FragColor.rgb);
    FragBrightness = (brightness > uBloomHdrThreshold)
        ? vec4(FragColor.rgb, brightness)
        : vec4(0.0, 0.0, 0.0, brightness);
}
