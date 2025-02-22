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

uniform sampler2D uTexSSAO;
uniform sampler2D uTexAlbedo;
uniform sampler2D uTexEmission;

uniform float uBloomHdrThreshold;

/* === Fragments === */

out vec3 FragColor;
out vec3 FragBrightness;

/* === Helper functions === */

float GetBrightness(vec3 color)
{
    return length(color);
}

/* === Main function === */

void main()
{
    vec3 envAmbient = texture(uTexEnvAmbient, vTexCoord).rgb;
    vec3 envSpecular = texture(uTexEnvSpecular, vTexCoord).rgb;

    vec3 objDiffuse = texture(uTexObjDiffuse, vTexCoord).rgb;
    vec3 objSpecular = texture(uTexObjSpecular, vTexCoord).rgb;

    float ssao = texture(uTexSSAO, vTexCoord).r;
    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb;
    vec3 emission = texture(uTexEmission, vTexCoord).rgb;

    // NOTE: Despite the fact that the occlusion map is applied only to "real" lighting,
    //       I have chosen to apply SSAO to both the lighting and the environment.
    //       This means that with extreme values, the rendering can become black
    //       (darker than the ambient tint).
    //       This is not physically correct, but it somewhat better simulates
    //       "global illumination" in a way...

    vec3 diffuse = albedo * (envAmbient + objDiffuse) * ssao;
    vec3 specular = (objSpecular + envSpecular) * ssao;

    FragColor = diffuse + specular + emission;

    float bright = GetBrightness(FragColor);
    FragBrightness = FragColor * step(uBloomHdrThreshold, bright);
}
