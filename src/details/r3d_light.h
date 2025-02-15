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

#ifndef R3D_LIGHT_H
#define R3D_LIGHT_H

#include <raylib.h>

typedef struct {
    Vector3 color;
    Vector3 position;
    Vector3 direction;
    float energy;
    float range;
    float attenuation;
    float innerCutOff;
    float outerCutOff;
    R3D_LightType type;
    bool enabled;
} r3d_light_t;

static inline void r3d_light_init(r3d_light_t* light)
{
    light->color = (Vector3) { 1, 1, 1 };
	light->position = (Vector3) { 0 };
    light->direction = (Vector3) { 0, 0, -1 };
    light->energy = 1.0f;
    light->range = 100.0f;
    light->attenuation = 1.0f;
    light->innerCutOff = -1.0f;
    light->outerCutOff = -1.0f;
    light->type = R3D_LIGHT_DIR;
	light->enabled = false;
}

#endif // R3D_LIGHT_H
