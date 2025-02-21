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
    R3D_FLAG_NONE           = 0,
    R3D_FLAG_FXAA           = 1 << 0,
    R3D_FLAG_BLIT_LINEAR    = 1 << 1,
    R3D_FLAG_ASPECT_KEEP    = 1 << 2,
} R3D_Flags;

typedef enum {
    R3D_RENDER_DEFERRED     = 0,        ///< More efficient on desktop GPUs but does not support transparency
    R3D_RENDER_FORWARD      = 1         ///< More efficient on tile-based rendering devices, supports transparency
} R3D_RenderMode;

typedef enum {
    R3D_SORT_AUTO           = 0,        ///< R3D will decide what to sort for you
    R3D_SORT_DISABLED       = 1,        ///< No sorting will be applied to draw calls
    R3D_SORT_FRONT_TO_BACK  = 2,        ///< Can optimize the depth testing phase
    R3D_SORT_BACK_TO_FRONT  = 3         ///< Less optimized but suitable when rendering transparency in forward mode
} R3D_SortMode;

typedef enum {
    R3D_LIGHT_DIR,
    R3D_LIGHT_SPOT,
    R3D_LIGHT_OMNI,
} R3D_LightType;

typedef enum {
    R3D_SHADOW_UPDATE_MANUAL,           ///< Manual shadow map update  
    R3D_SHADOW_UPDATE_INTERVAL,         ///< Update at regular intervals  
    R3D_SHADOW_UPDATE_CONTINUOUS        ///< Continuous update  
} R3D_ShadowUpdateMode;

typedef enum {
    R3D_BLOOM_DISABLED,                 ///< Bloom effect is disabled.
    R3D_BLOOM_ADDITIVE,                 ///< Additive bloom effect, where bright areas are enhanced by adding light to them.
    R3D_BLOOM_SOFT_LIGHT                ///< Soft light bloom effect, which creates a softer, more diffused glow around bright areas.
} R3D_Bloom;

typedef enum {
    R3D_FOG_DISABLED,                   ///< Fog effect is disabled.
    R3D_FOG_LINEAR,                     ///< Linear fog, where the density increases linearly based on distance from the camera.
    R3D_FOG_EXP2,                       ///< Exponential fog (exp2), where the density increases exponentially with distance.
    R3D_FOG_EXP,                        ///< Exponential fog, where the density increases exponentially but at a different rate compared to EXP2.
} R3D_Fog;

typedef enum {
    R3D_TONEMAP_LINEAR,                 ///< Linear tone mapping, which performs a simple linear mapping of HDR values.
    R3D_TONEMAP_REINHARD,               ///< Reinhard tone mapping, a popular algorithm for compressing HDR values.
    R3D_TONEMAP_FILMIC,                 ///< Filmic tone mapping, which simulates the response of film to light.
    R3D_TONEMAP_ACES,                   ///< ACES (Academy Color Encoding System) tone mapping, a high-quality algorithm used for cinematic rendering.
} R3D_Tonemap;


/* === Types === */

typedef unsigned int R3D_Light;

typedef struct {
    TextureCubemap cubemap;             ///< The skybox cubemap texture for the background and reflections.
    Texture2D irradiance;               ///< The irradiance cubemap for diffuse lighting (ambient light).
    Texture2D prefilter;                ///< The prefiltered cubemap for specular reflections with mipmaps.
} R3D_Skybox;


/* === Core functions === */

void R3D_Init(int resWidth, int resHeight, unsigned int flags);
void R3D_Close(void);

bool R3D_HasState(unsigned int flag);
void R3D_SetState(unsigned int flags);
void R3D_ClearState(unsigned int flags);

void R3D_GetResolution(int* width, int* height);
void R3D_UpdateResolution(int width, int height);

void R3D_EnableCustomTarget(RenderTexture target);
void R3D_DisableCustomTarget(void);

R3D_SortMode R3D_GetSortMode(R3D_RenderMode renderMode);
void R3D_SetSortMode(R3D_RenderMode renderMode, R3D_SortMode mode);

void R3D_Begin(Camera3D camera);
void R3D_End(void);

void R3D_DrawMesh(Mesh mesh, Material material, Matrix transform);
void R3D_DrawMeshInstanced(Mesh mesh, Material material, Matrix* instanceTransforms, int instanceCount);
void R3D_DrawMeshInstancedEx(Mesh mesh, Material material, Matrix* instanceTransforms, Color* instanceColors, int instanceCount);
void R3D_DrawMeshInstancedPro(Mesh mesh, Material material, Matrix transform, Matrix* instanceTransforms, Color* instanceColors, int instanceCount);

void R3D_DrawModel(Model model, Vector3 position, float scale);
void R3D_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale);


/* === Lighting functions === */

R3D_Light R3D_CreateLight(R3D_LightType type);
void R3D_DestroyLight(R3D_Light id);
bool R3D_IsLightExist(R3D_Light id);

R3D_LightType R3D_GetLightType(R3D_Light id);

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

void R3D_EnableShadow(R3D_Light id, int resolution);
void R3D_DisableShadow(R3D_Light id, bool destroyMap);

bool R3D_IsShadowEnabled(R3D_Light id);
bool R3D_HasShadowMap(R3D_Light id);

R3D_ShadowUpdateMode R3D_GetShadowUpdateMode(R3D_Light id);
void R3D_SetShadowUpdateMode(R3D_Light id, R3D_ShadowUpdateMode mode);

int R3D_GetShadowUpdateFrequency(R3D_Light id);
void R3D_SetShadowUpdateFrequency(R3D_Light id, int msec);

void R3D_UpdateShadowMap(R3D_Light id);

float R3D_GetShadowBias(R3D_Light id);
void R3D_SetShadowBias(R3D_Light id, float value);

void R3D_DrawLightShape(R3D_Light id);


/* === Environment functions === */

void R3D_SetBackgroundColor(Color color);
void R3D_SetAmbientColor(Color color);

void R3D_EnableSkybox(R3D_Skybox skybox);
void R3D_DisableSkybox(void);

void R3D_SetSkyboxRotation(float pitch, float yaw, float roll);
Vector3 R3D_GetSkyboxRotation(void);

void R3D_SetSSAO(bool enabled);
bool R3D_GetSSAO(void);

void R3D_SetSSAORadius(float value);
float R3D_GetSSAORadius(void);

void R3D_SetSSAOBias(float value);
float R3D_GetSSAOBias(void);

void R3D_SetSSAOIterations(int value);
int R3D_GetSSAOIterations(void);

void R3D_SetBloomMode(R3D_Bloom mode);
R3D_Bloom R3D_GetBloomMode(void);

void R3D_SetBloomIntensity(float value);
float R3D_GetBloomIntensity(void);

void R3D_SetBloomHdrThreshold(float value);
float R3D_GetBloomHdrThreshold(void);

void R3D_SetBloomIterations(int value);
int R3D_GetBloomIterations(void);

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


/* === Culling functions === */

bool R3D_IsPointInFrustum(Vector3 position);
bool R3D_IsPointInFrustumXYZ(float x, float y, float z);
bool R3D_IsSphereInFrustum(Vector3 position, float radius);
bool R3D_IsBoundingBoxInFrustum(BoundingBox aabb);


/* === Utils functions === */

void R3D_SetMaterialAlbedo(Material* material, Texture2D* texture, Color color);
void R3D_SetMaterialOcclusion(Material* material, Texture2D* texture, float value);
void R3D_SetMaterialRoughness(Material* material, Texture2D* texture, float value);
void R3D_SetMaterialMetalness(Material* material, Texture2D* texture, float value);
void R3D_SetMaterialEmission(Material* material, Texture2D* texture, Color color, float value);

Texture2D R3D_GetWhiteTexture(void);
Texture2D R3D_GetBlackTexture(void);
Texture2D R3D_GetNormalTexture(void);

void R3D_DrawBufferAlbedo(float x, float y, float w, float h);
void R3D_DrawBufferEmission(float x, float y, float w, float h);
void R3D_DrawBufferNormal(float x, float y, float w, float h);
void R3D_DrawBufferORM(float x, float y, float w, float h);
void R3D_DrawBufferSSAO(float x, float y, float w, float h);
void R3D_DrawBufferBrightColors(float x, float y, float w, float h);
void R3D_DrawBufferBloom(float x, float y, float w, float h);


#endif // R3D_H
