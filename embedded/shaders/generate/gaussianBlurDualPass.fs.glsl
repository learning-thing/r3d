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

// NOTE: The coefficients for the two-pass Gaussian blur were generated using:
//       https://lisyarus.github.io/blog/posts/blur-coefficients-generator.html

#version 330 core

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uDirection;
out vec4 FragColor;

/* === Blur Coefs === */

const int SAMPLE_COUNT = 5;

const float OFFSETS[5] = float[5](
    -3.4048471718931532,
    -1.4588111840004858,
    0.48624268466894843,
    2.431625915613778,
    4
);

const float WEIGHTS[5] = float[5](
    0.15642123799829394,
    0.26718801880015064,
    0.29738065394682034,
    0.21568339342709997,
    0.06332669582763516
);

/* === Main Program === */

void main()
{
    vec3 result = vec3(0.0);
    vec2 size = textureSize(uTexture, 0);

    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 offset = uDirection * OFFSETS[i] / size;
        result += texture(uTexture, vTexCoord + offset).rgb * WEIGHTS[i];
    }

    FragColor = vec4(result, 1.0);
}
