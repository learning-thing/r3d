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

uniform sampler2D uTexEnvAmbient;
uniform sampler2D uTexEnvSpecular;

uniform sampler2D uTexObjDiffuse;
uniform sampler2D uTexObjSpecular;

uniform sampler2D uTexORM;          //< Just for occlusion map (from G-Buffer)
uniform sampler2D uTexSSAO;
uniform sampler2D uTexAlbedo;
uniform sampler2D uTexEmission;

uniform float uBloomHdrThreshold;

/* === Fragments === */

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec3 FragBrightness;

/* === Helper functions === */

float GetBrightness(vec3 color)
{
    return length(color);
}

/* === Main function === */

void main()
{
    /* Sample textures */

    vec3 envAmbient = texture(uTexEnvAmbient, vTexCoord).rgb;
    vec3 envSpecular = texture(uTexEnvSpecular, vTexCoord).rgb;

    vec3 objDiffuse = texture(uTexObjDiffuse, vTexCoord).rgb;
    vec3 objSpecular = texture(uTexObjSpecular, vTexCoord).rgb;

    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;
    vec3 emission = texture(uTexEmission, vTexCoord).rgb;

    float occlusion = texture(uTexORM, vTexCoord).r;
    occlusion *= texture(uTexSSAO, vTexCoord).r;

    /* Compute ambient lighting and direct lighting */

    envAmbient *= occlusion;

    vec3 diffuse = albedo * (envAmbient + objDiffuse);
    vec3 specular = (objSpecular + envSpecular);

    FragColor = diffuse + specular + emission;

    /* Extract brightness parts for bloom */

    float bright = GetBrightness(FragColor);
    FragBrightness = FragColor * step(uBloomHdrThreshold, bright);
}
