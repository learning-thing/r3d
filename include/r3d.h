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


/* === Types === */

typedef enum {
    R3D_LIGHT_DIR,
    R3D_LIGHT_SPOT,
    R3D_LIGHT_OMNI,
} R3D_LightType;

typedef unsigned int R3D_Light;

typedef struct {
    TextureCubemap cubemap;     ///< The skybox cubemap texture for the background and reflections.
    Texture2D irradiance;       ///< The irradiance cubemap for diffuse lighting (ambient light).
    Texture2D prefilter;        ///< The prefiltered cubemap for specular reflections with mipmaps.
} R3D_Skybox;

typedef enum {
    R3D_PLANE_BACK = 0,
    R3D_PLANE_FRONT,
    R3D_PLANE_BOTTOM,
    R3D_PLANE_TOP,
    R3D_PLANE_RIGHT,
    R3D_PLANE_LEFT,
    R3D_PLANE_COUNT
} R3D_Plane;

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
