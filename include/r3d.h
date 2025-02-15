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

#ifndef R3D_H
#define R3D_H

#include <raylib.h>
#include <rlgl.h>


/* === Enums === */

typedef enum {
    R3D_BLOOM_DISABLED,         ///< Bloom effect is disabled.
    R3D_BLOOM_ADDITIVE,         ///< Additive bloom effect, where bright areas are enhanced by adding light to them.
    R3D_BLOOM_SOFT_LIGHT        ///< Soft light bloom effect, which creates a softer, more diffused glow around bright areas.
} R3D_Bloom;

typedef enum {
    R3D_FOG_DISABLED,           ///< Fog effect is disabled.
    R3D_FOG_LINEAR,             ///< Linear fog, where the density increases linearly based on distance from the camera.
    R3D_FOG_EXP2,               ///< Exponential fog (exp2), where the density increases exponentially with distance.
    R3D_FOG_EXP,                ///< Exponential fog, where the density increases exponentially but at a different rate compared to EXP2.
} R3D_Fog;

typedef enum {
    R3D_TONEMAP_LINEAR,         ///< Linear tone mapping, which performs a simple linear mapping of HDR values.
    R3D_TONEMAP_REINHARD,       ///< Reinhard tone mapping, a popular algorithm for compressing HDR values.
    R3D_TONEMAP_FILMIC,         ///< Filmic tone mapping, which simulates the response of film to light.
    R3D_TONEMAP_ACES,           ///< ACES (Academy Color Encoding System) tone mapping, a high-quality algorithm used for cinematic rendering.
} R3D_Tonemap;

typedef enum {
    R3D_PLANE_BACK = 0,
    R3D_PLANE_FRONT,
    R3D_PLANE_BOTTOM,
    R3D_PLANE_TOP,
    R3D_PLANE_RIGHT,
    R3D_PLANE_LEFT,
    R3D_PLANE_COUNT
} R3D_Plane;


/* === Types === */

typedef unsigned int R3D_Light;

typedef struct {
    TextureCubemap cubemap;     ///< The skybox cubemap texture for the background and reflections.
    Texture2D irradiance;       ///< The irradiance cubemap for diffuse lighting (ambient light).
    Texture2D prefilter;        ///< The prefiltered cubemap for specular reflections with mipmaps.
} R3D_Skybox;

typedef enum {
    R3D_LIGHT_DIR,
    R3D_LIGHT_SPOT,
    R3D_LIGHT_OMNI,
} R3D_LightType;

typedef struct {
    Vector4 planes[R3D_PLANE_COUNT];
} R3D_Frustum;


/* === Core functions === */

void R3D_Init(int resWidth, int resHeight);
void R3D_Close(void);

void R3D_Begin(Camera3D camera);
void R3D_End(void);

void R3D_DrawMesh(Mesh mesh, Material material, Matrix transform);

void R3D_DrawModel(Model model, Vector3 position, float scale);
void R3D_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale);


/* === Lighting functions === */

R3D_Light R3D_CreateLight(void);
void R3D_DestroyLight(R3D_Light id);
bool R3D_IsLightExist(R3D_Light id);

bool R3D_IsLightActive(R3D_Light id);
void R3D_ToggleLight(R3D_Light id);
void R3D_SetLightActive(R3D_Light id, bool active);

Color R3D_GetLightColor(R3D_Light id);
Vector3 R3D_GetLightColorV(R3D_Light id);
void R3D_SetLightColor(R3D_Light id, Color color);
void R3D_SetLightColorV(R3D_Light id, Vector3 color);

Vector3 R3D_GetLightPosition(R3D_Light id);
void R3D_SetLightPosition(R3D_Light id, Vector3 position);

Vector3 R3D_GetLightDirection(R3D_Light id);
void R3D_SetLightDirection(R3D_Light id, Vector3 direction);
void R3D_SetLightTarget(R3D_Light id, Vector3 target);

float R3D_GetLightEnergy(R3D_Light id);
void R3D_SetLightEnergy(R3D_Light id, float energy);

float R3D_GetLightRange(R3D_Light id);
void R3D_SetLightRange(R3D_Light id, float range);

float R3D_GetLightAttenuation(R3D_Light id);
void R3D_SetLightAttenuation(R3D_Light id, float attenuation);

float R3D_GetLightInnerCutOff(R3D_Light id);
void R3D_SetLightInnerCutOff(R3D_Light id, float degrees);

float R3D_GetLightOuterCutOff(R3D_Light id);
void R3D_SetLightOuterCutOff(R3D_Light id, float degrees);

R3D_LightType R3D_GetLightType(R3D_Light id);
void R3D_SetLightType(R3D_Light id, R3D_LightType type);


/* === Environment functions === */

void R3D_SetBackgroundColor(Color color);
void R3D_SetAmbientColor(Color color);

void R3D_EnableSkybox(R3D_Skybox skybox);
void R3D_DisableSkybox(void);

void R3D_SetBloom(R3D_Bloom mode);
R3D_Bloom R3D_GetBloom(void);

void R3D_SetBloomIntensity(float value);
float R3D_GetBloomIntensity(void);

void R3D_SetBloomHdrThreshold(float value);
float R3D_GetBloomHdrThreshold(void);

void R3D_SetFogMode(R3D_Fog mode);
R3D_Fog R3D_GetFogMode(void);

void R3D_SetFogColor(Color color);
Color R3D_GetFogColor(void);

void R3D_SetFogStart(float value);
float R3D_GetFogStart(void);

void R3D_SetFogEnd(float value);
float R3D_GetFogEnd(void);

void R3D_SetFogDensity(float value);
float R3D_GetFogDensity(void);

void R3D_SetTonemapMode(R3D_Tonemap mode);
R3D_Tonemap R3D_GetTonemapMode(void);

void R3D_SetTonemapExposure(float value);
float R3D_GetTonemapExposure(void);

void R3D_SetTonemapWhite(float value);
float R3D_GetTonemapWhite(void);

void R3D_SetBrightness(float value);
float R3D_GetBrightness(void);

void R3D_SetContrast(float value);
float R3D_GetContrast(void);

void R3D_SetSaturation(float value);
float R3D_GetSaturation(void);


/* === Skybox functions === */

R3D_Skybox R3D_LoadSkybox(const char* fileName, CubemapLayout layout);
R3D_Skybox R3D_LoadSkyboxHDR(const char* fileName, int size);
void R3D_UnloadSkybox(R3D_Skybox sky);


/* === Frustum functions === */

R3D_Frustum R3D_GetFrustum(void);
R3D_Frustum R3D_CreateFrustum(Matrix matrixViewProjection);
bool R3D_IsPointInFrustum(const R3D_Frustum* frustum, Vector3 position);
bool R3D_IsPointInFrustumXYZ(const R3D_Frustum* frustum, float x, float y, float z);
bool R3D_IsSphereInFrustum(const R3D_Frustum* frustum, Vector3 position, float radius);
bool R3D_IsBoundingBoxInFrustum(const R3D_Frustum* frustum, BoundingBox aabb);


/* === Debug functions === */

void R3D_DrawBufferAlbedo(float x, float y, float w, float h);
void R3D_DrawBufferNormal(float x, float y, float w, float h);
void R3D_DrawBufferORM(float x, float y, float w, float h);

#endif // R3D_H
