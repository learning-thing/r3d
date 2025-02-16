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

#ifndef R3D_EMBEDDED_SHADERS_H
#define R3D_EMBEDDED_SHADERS_H

#include <raylib.h>


/* === Shader defines === */

#define R3D_SHADER_NUM_LIGHTS 8


/* === Shader code declarations === */

extern const char VS_COMMON_SCREEN[];
extern const char VS_COMMON_CUBEMAP[];

extern const char FS_GENERATE_GAUSSIAN_BLUR_DUAL_PASS[];
extern const char FS_GENERATE_CUBEMAP_FROM_EQUIRECTANGULAR[];
extern const char FS_GENERATE_IRRADIANCE_CONVOLUTION[];
extern const char FS_GENERATE_PREFILTER[];

extern const char VS_RASTER_GEOMETRY[];
extern const char FS_RASTER_GEOMETRY[];
extern const char VS_RASTER_SKYBOX[];
extern const char FS_RASTER_SKYBOX[];

extern const char FS_SCREEN_LIGHTING[];
extern const char FS_SCREEN_BLOOM[];
extern const char FS_SCREEN_FOG[];
extern const char FS_SCREEN_TONEMAP[];
extern const char FS_SCREEN_ADJUSTMENT[];

/* === Uniform types === */

typedef struct { int val; int loc; } r3d_shader_uniform_int_t;
typedef struct { float val; int loc; } r3d_shader_uniform_float_t;
typedef struct { Vector2 val; int loc; } r3d_shader_uniform_vec2_t;
typedef struct { Vector3 val; int loc; } r3d_shader_uniform_vec3_t;
typedef struct { Vector4 val; int loc; } r3d_shader_uniform_vec4_t;
typedef struct { int loc; } r3d_shader_uniform_mat4_t;


/* === Shader struct definitions === */

typedef struct {
    unsigned int id;
    r3d_shader_uniform_int_t uTexture;
    r3d_shader_uniform_vec2_t uDirection;
} r3d_shader_generate_gaussian_blur_dual_pass_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_mat4_t uMatProj;
    r3d_shader_uniform_mat4_t uMatView;
    r3d_shader_uniform_int_t uTexEquirectangular;
} r3d_shader_generate_cubemap_from_equirectangular_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_mat4_t uMatProj;
    r3d_shader_uniform_mat4_t uMatView;
    r3d_shader_uniform_int_t uTexCubemap;
} r3d_shader_generate_irradiance_convolution_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_mat4_t uMatProj;
    r3d_shader_uniform_mat4_t uMatView;
    r3d_shader_uniform_int_t uTexCubemap;
    r3d_shader_uniform_float_t uRoughness;
} r3d_shader_generate_prefilter_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_mat4_t uMatNormal;
    r3d_shader_uniform_mat4_t uMatModel;
    r3d_shader_uniform_mat4_t uMatMVP;
    r3d_shader_uniform_int_t uTexAlbedo;
    r3d_shader_uniform_int_t uTexNormal;
    r3d_shader_uniform_int_t uTexEmission;
    r3d_shader_uniform_int_t uTexOcclusion;
    r3d_shader_uniform_int_t uTexRoughness;
    r3d_shader_uniform_int_t uTexMetalness;
    r3d_shader_uniform_float_t uValEmission;
    r3d_shader_uniform_float_t uValOcclusion;
    r3d_shader_uniform_float_t uValRoughness;
    r3d_shader_uniform_float_t uValMetalness;
    r3d_shader_uniform_vec3_t uColAlbedo;
    r3d_shader_uniform_vec3_t uColEmission;
} r3d_shader_raster_geometry_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_mat4_t uMatProj;
    r3d_shader_uniform_mat4_t uMatView;
    r3d_shader_uniform_vec4_t uRotation;
    r3d_shader_uniform_int_t uTexSkybox;
} r3d_shader_raster_skybox_t;

typedef struct {
    unsigned int id;
    struct {
        r3d_shader_uniform_vec3_t color;
        r3d_shader_uniform_vec3_t position;
        r3d_shader_uniform_vec3_t direction;
        r3d_shader_uniform_float_t energy;
        r3d_shader_uniform_float_t range;
        r3d_shader_uniform_float_t attenuation;
        r3d_shader_uniform_float_t innerCutOff;
        r3d_shader_uniform_float_t outerCutOff;
        r3d_shader_uniform_int_t type;
        r3d_shader_uniform_int_t enabled;
    } uLights[R3D_SHADER_NUM_LIGHTS];
    r3d_shader_uniform_int_t uTexAlbedo;
    r3d_shader_uniform_int_t uTexEmission;
    r3d_shader_uniform_int_t uTexNormal;
    r3d_shader_uniform_int_t uTexDepth;
    r3d_shader_uniform_int_t uTexORM;
    r3d_shader_uniform_int_t uTexID;
    r3d_shader_uniform_vec3_t uColAmbient;
    r3d_shader_uniform_int_t uCubeIrradiance;
    r3d_shader_uniform_int_t uCubePrefilter;
    r3d_shader_uniform_int_t uTexBrdfLut;
    r3d_shader_uniform_vec4_t uQuatSkybox;
    r3d_shader_uniform_int_t uHasSkybox;
    r3d_shader_uniform_float_t uBloomHdrThreshold;
    r3d_shader_uniform_vec3_t uViewPosition;
    r3d_shader_uniform_mat4_t uMatInvProj;
    r3d_shader_uniform_mat4_t uMatInvView;
} r3d_shader_screen_lighting_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_int_t uTexSceneHDR;
    r3d_shader_uniform_int_t uTexBloomBlurHDR;
    r3d_shader_uniform_int_t uBloomMode;
    r3d_shader_uniform_float_t uBloomIntensity;
} r3d_shader_screen_bloom_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_int_t uTexSceneHDR;
    r3d_shader_uniform_int_t uTexSceneDepth;
    r3d_shader_uniform_float_t uNear;
    r3d_shader_uniform_float_t uFar;
    r3d_shader_uniform_int_t uFogMode;
    r3d_shader_uniform_vec3_t uFogColor;
    r3d_shader_uniform_float_t uFogStart;
    r3d_shader_uniform_float_t uFogEnd;
    r3d_shader_uniform_float_t uFogDensity;
} r3d_shader_screen_fog_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_int_t uTexSceneHDR;
    r3d_shader_uniform_int_t uTonemapMode;
    r3d_shader_uniform_float_t uTonemapExposure;
    r3d_shader_uniform_float_t uTonemapWhite;
} r3d_shader_screen_tonemap_t;

typedef struct {
    unsigned int id;
    r3d_shader_uniform_int_t uTexSceneHDR;
    r3d_shader_uniform_float_t uBrightness;
    r3d_shader_uniform_float_t uContrast;
    r3d_shader_uniform_float_t uSaturation;
} r3d_shader_screen_adjustment_t;

#endif // R3D_EMBEDDED_SHADERS_H
