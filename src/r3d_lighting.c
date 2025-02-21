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

#include "r3d.h"

#include "./details/r3d_light.h"
#include "./r3d_state.h"

#include <raylib.h>
#include <raymath.h>


/* === Helper macros === */

#define r3d_get_and_check_light(var_name, id, ...)                  \
    r3d_light_t* var_name;                                          \
{                                                                   \
    var_name = r3d_registry_get(&R3D.container.rLights, id);  \
    if (light == NULL) {                                            \
        TraceLog(LOG_ERROR, "Light [ID %i] is not valid", id);      \
        return __VA_ARGS__;                                         \
    }                                                               \
}


/* === Public functions === */

R3D_Light R3D_CreateLight(R3D_LightType type)
{
    R3D_Light id = r3d_registry_add(&R3D.container.rLights, NULL);
    r3d_light_t* light = r3d_registry_get(&R3D.container.rLights, id);

    r3d_light_init(light);
    light->type = type;

    // Set default shadow map config
    light->shadow.updateConf.mode = R3D_SHADOW_UPDATE_INTERVAL;
    light->shadow.updateConf.frequencySec = 0.016f;
    light->shadow.updateConf.timerSec = 0.0f;
    light->shadow.updateConf.shoudlUpdate = true;

    // Set default shadow bias
    switch (type) {
    case R3D_LIGHT_DIR:
    case R3D_LIGHT_SPOT:
        light->shadow.bias = 0.0002f;
        break;
    case R3D_LIGHT_OMNI:
        light->shadow.bias = 0.05f;
        break;
    }

    return id;
}

void R3D_DestroyLight(R3D_Light id)
{
    r3d_get_and_check_light(light, id);

    if (light->shadow.map.id != 0) {
        r3d_light_destroy_shadow_map(light);
    }

    r3d_registry_remove(&R3D.container.rLights, id);
}

bool R3D_IsLightExist(R3D_Light id)
{
    r3d_get_and_check_light(light, id, false);
    return true;
}

R3D_LightType R3D_GetLightType(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->type;
}

bool R3D_IsLightActive(R3D_Light id)
{
    r3d_get_and_check_light(light, id, false);
    return light->enabled;
}

void R3D_ToggleLight(R3D_Light id)
{
    r3d_get_and_check_light(light, id);
    light->enabled = !light->enabled;

    if (light->enabled && light->shadow.enabled) {
        light->shadow.updateConf.shoudlUpdate = true;
    }
}

void R3D_SetLightActive(R3D_Light id, bool active)
{
    r3d_get_and_check_light(light, id);

    if (light->enabled == active) {
        return;
    }

    if (active && light->shadow.enabled) {
        light->shadow.updateConf.shoudlUpdate = true;
    }

    light->enabled = active;
}

Color R3D_GetLightColor(R3D_Light id)
{
    r3d_get_and_check_light(light, id, BLANK);
    return (Color) {
        (unsigned char)Clamp(light->color.x * 255, 0, 255),
        (unsigned char)Clamp(light->color.y * 255, 0, 255),
        (unsigned char)Clamp(light->color.z * 255, 0, 255),
        255
    };
}

Vector3 R3D_GetLightColorV(R3D_Light id)
{
    r3d_get_and_check_light(light, id, (Vector3) { 0 });
    return light->color;
}

void R3D_SetLightColor(R3D_Light id, Color color)
{
    r3d_get_and_check_light(light, id);
    light->color.x = color.r / 255.0f;
    light->color.y = color.g / 255.0f;
    light->color.z = color.b / 255.0f;
}

void R3D_SetLightColorV(R3D_Light id, Vector3 color)
{
    r3d_get_and_check_light(light, id);
    light->color = color;
}

Vector3 R3D_GetLightPosition(R3D_Light id)
{
    r3d_get_and_check_light(light, id, (Vector3) { 0 });
    return light->position;
}

void R3D_SetLightPosition(R3D_Light id, Vector3 position)
{
    r3d_get_and_check_light(light, id);
    light->position = position;
}

Vector3 R3D_GetLightDirection(R3D_Light id)
{
    r3d_get_and_check_light(light, id, (Vector3) { 0 });
    return light->direction;
}

void R3D_SetLightDirection(R3D_Light id, Vector3 direction)
{
    r3d_get_and_check_light(light, id);
    light->direction = Vector3Normalize(direction);
}

void R3D_SetLightTarget(R3D_Light id, Vector3 target)
{
    r3d_get_and_check_light(light, id);
    light->direction = Vector3Normalize(Vector3Subtract(target, light->position));
}

float R3D_GetLightEnergy(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->energy;
}

void R3D_SetLightEnergy(R3D_Light id, float energy)
{
    r3d_get_and_check_light(light, id);
    light->energy = energy;
}

float R3D_GetLightRange(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->range;
}

void R3D_SetLightRange(R3D_Light id, float range)
{
    r3d_get_and_check_light(light, id);
    light->range = range;
}

float R3D_GetLightAttenuation(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->attenuation;
}

void R3D_SetLightAttenuation(R3D_Light id, float attenuation)
{
    r3d_get_and_check_light(light, id);
    light->attenuation = attenuation;
}

float R3D_GetLightInnerCutOff(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return acosf(light->innerCutOff) * RAD2DEG;
}

void R3D_SetLightInnerCutOff(R3D_Light id, float degrees)
{
    r3d_get_and_check_light(light, id);
    light->innerCutOff = cosf(degrees * DEG2RAD);
}

float R3D_GetLightOuterCutOff(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return acosf(light->outerCutOff) * RAD2DEG;
}

void R3D_SetLightOuterCutOff(R3D_Light id, float degrees)
{
    r3d_get_and_check_light(light, id);
    light->outerCutOff = cosf(degrees * DEG2RAD);
}

void R3D_EnableShadow(R3D_Light id, int resolution)
{
    r3d_get_and_check_light(light, id);

    if (light->shadow.map.id != 0) {
        if (resolution > 0 && light->shadow.map.resolution != resolution) {
            r3d_light_destroy_shadow_map(light);
            r3d_light_create_shadow_map(light, resolution);
        }
    }
    else {
        if (resolution == 0) resolution = 1024;
        r3d_light_create_shadow_map(light, resolution);
    }

    light->shadow.enabled = true;
}

void R3D_DisableShadow(R3D_Light id, bool destroyMap)
{
    r3d_get_and_check_light(light, id);

    if (destroyMap) {
        r3d_light_destroy_shadow_map(light);
        memset(&light->shadow.map, 0, sizeof(r3d_shadow_map_t));
    }

    light->shadow.enabled = false;
}

bool R3D_IsShadowEnabled(R3D_Light id)
{
    r3d_get_and_check_light(light, id, false);
    return light->shadow.enabled;
}

bool R3D_HasShadowMap(R3D_Light id)
{
    r3d_get_and_check_light(light, id, false);
    return light->shadow.map.id != 0;
}

R3D_ShadowUpdateMode R3D_GetShadowUpdateMode(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->shadow.updateConf.mode;
}

void R3D_SetShadowUpdateMode(R3D_Light id, R3D_ShadowUpdateMode mode)
{
    r3d_get_and_check_light(light, id);
    light->shadow.updateConf.mode = mode;
}

int R3D_GetShadowUpdateFrequency(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->shadow.updateConf.frequencySec * 1000;
}

void R3D_SetShadowUpdateFrequency(R3D_Light id, int msec)
{
    r3d_get_and_check_light(light, id);
    light->shadow.updateConf.frequencySec = (float)msec / 1000;

}

void R3D_UpdateShadowMap(R3D_Light id)
{
    r3d_get_and_check_light(light, id);
    light->shadow.updateConf.shoudlUpdate = true;
}

float R3D_GetShadowBias(R3D_Light id)
{
    r3d_get_and_check_light(light, id, 0);
    return light->shadow.bias;
}

void R3D_SetShadowBias(R3D_Light id, float value)
{
    r3d_get_and_check_light(light, id);
    light->shadow.bias = value;
}
