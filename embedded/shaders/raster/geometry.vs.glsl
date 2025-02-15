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

/* === Attributes === */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec4 aTangent;

/* === Uniforms === */

uniform mat4 uMatNormal;
uniform mat4 uMatModel;
uniform mat4 uMatMVP;

/* === Varyings === */

out vec3 vPosition;
out vec2 vTexCoord;
out vec3 vNormal;
out vec4 vColor;
out mat3 vTBN;

/* === Main function === */

void main()
{
    vPosition = aPosition;
    vTexCoord = aTexCoord;
    vColor = aColor;

    vNormal = normalize(vec3(uMatNormal * vec4(aNormal, 1.0)));

    // The TBN matrix is used to transform vectors from tangent space to world space
    // It is currently used to transform normals from a normal map to world space normals
    vec3 T = normalize(vec3(uMatModel * vec4(aTangent.xyz, 0.0)));
    vec3 B = cross(vNormal, T) * aTangent.w;
    vTBN = mat3(T, B, vNormal);

    gl_Position = uMatMVP * vec4(aPosition, 1.0);
}
