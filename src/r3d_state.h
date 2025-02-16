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

#ifndef R3D_STATE_H
#define R3D_STATE_H

#include "r3d.h"

#include "./details/r3d_primitives.h"
#include "./details/containers/r3d_array.h"
#include "./details/containers/r3d_registry.h"

#include "./embedded/r3d_shaders.h"
#include "./embedded/r3d_textures.h"


/* === Global r3d state === */

extern struct R3D_State {

    // Framebuffers
    struct {

        // G-Buffer
        struct r3d_fb_gbuffer_t {
            unsigned int id;
            unsigned int albedo;
            unsigned int emission;
            unsigned int normal;
            unsigned int orm;
            unsigned int matId;
            unsigned int depth;
        } gBuffer;

        // Lit scene
        struct r3d_fb_lit_t {
            unsigned int id;
            unsigned int color;
            unsigned int lum;
        } lit;

        // Post process
        struct r3d_fb_post_t {
            unsigned int id;
            unsigned int textures[2];
        } post;

        // Custom target (optional)
        RenderTexture customTarget;

    } framebuffer;

    // Containers
    struct {
        r3d_array_t drawCallArray;
        r3d_registry_t lightRegistry;
    } container;

    // Internal shaders
    struct {

        // Generation shaders
        struct {
            r3d_shader_generate_cubemap_from_equirectangular_t cubemapFromEquirectangular;
            r3d_shader_generate_irradiance_convolution_t irradianceConvolution;
            r3d_shader_generate_prefilter_t prefilter;
        } generate;

        // Raster shaders
        struct {
            r3d_shader_raster_geometry_t geometry;
            r3d_shader_raster_skybox_t skybox;
        } raster;

        // Screen shaders
        struct {
            r3d_shader_screen_lighting_t lighting;
            r3d_shader_screen_bloom_t bloom;
            r3d_shader_screen_fog_t fog;
            r3d_shader_screen_tonemap_t tonemap;
            r3d_shader_screen_adjustment_t adjustment;
        } screen;

    } shader;

    // Environment data
    struct {

        Vector3 backgroundColor;    // Used as default albedo color when skybox is disabled (raster pass)
        Vector3 ambientColor;       // Used as default ambient light when skybox is disabled (light pass)

        Quaternion quatSky;         // Rotation of the skybox (raster / light passes)
        R3D_Skybox sky;             // Skybox textures (raster / light passes)
        bool useSky;                // Flag to indicate if skybox is enabled (light pass)

        R3D_Bloom bloomMode;        // (post pass)
        float bloomIntensity;       // (post pass)
        float bloomHdrThreshold;    // (light pass)

        R3D_Fog fogMode;            // (post pass)
        Vector3 fogColor;           // (post pass)
        float fogStart;             // (post pass)
        float fogEnd;               // (post pass)
        float fogDensity;           // (post pass)

        R3D_Tonemap tonemapMode;    // (post pass)
        float tonemapExposure;      // (post pass)
        float tonemapWhite;         // (post pass)

        float brightness;           // (post pass)
        float contrast;             // (post pass)
        float saturation;           // (post pass)

    } env;

    // Default textures
    struct {
        unsigned int white;
        unsigned int black;
        unsigned int normal;
        unsigned int iblBrdfLut;
    } texture;

    // Primitives
    struct {
        r3d_primitive_t quad;
        r3d_primitive_t cube;
    } primitive;

    // State data
    struct {
        Vector3 posView;
        Matrix matView;
        Matrix matProj;
        int resolutionW;
        int resolutionH;
    } state;

    // Misc data
    struct {
        Matrix matCubeViews[6];
    } misc;

} R3D;


/* === Framebuffer loading functions === */

void r3d_framebuffer_load_gbuffer(int width, int height);
void r3d_framebuffer_load_lit(int width, int height);
void r3d_framebuffer_load_post(int width, int height);

void r3d_framebuffer_unload_gbuffer(void);
void r3d_framebuffer_unload_lit(void);
void r3d_framebuffer_unload_post(void);


/* === Shader loading functions === */

void r3d_shader_load_generate_cubemap_from_equirectangular(void);
void r3d_shader_load_generate_irradiance_convolution(void);
void r3d_shader_load_generate_prefilter(void);
void r3d_shader_load_raster_geometry(void);
void r3d_shader_load_raster_skybox(void);
void r3d_shader_load_screen_lighting(void);
void r3d_shader_load_screen_bloom(void);
void r3d_shader_load_screen_fog(void);
void r3d_shader_load_screen_tonemap(void);
void r3d_shader_load_screen_adjustment(void);


/* === Texture loading functions === */

void r3d_texture_load_white(void);
void r3d_texture_load_black(void);
void r3d_texture_load_normal(void);
void r3d_texture_load_ibl_brdf_lut(void);


/* === Shader helper macros === */

#define r3d_shader_enable(shader_name)                                                          \
{                                                                                               \
    rlEnableShader(R3D.shader.shader_name.id);                                                  \
}

#define r3d_shader_disable()                                                                    \
{                                                                                               \
    rlDisableShader();                                                                          \
}

#define r3d_shader_get_location(shader_name, uniform, value)                                    \
{                                                                                               \
    R3D.shader.shader_name.uniform.loc = rlGetLocationUniform(                                  \
        R3D.shader.shader_name.id, #uniform                                                     \
    );                                                                                          \
}

#define r3d_shader_set_int(shader_name, uniform, value)                                         \
{                                                                                               \
    if (R3D.shader.shader_name.uniform.val != value) {                                          \
        R3D.shader.shader_name.uniform.val = value;                                             \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_INT, 1                                                            \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_float(shader_name, uniform, value)                                       \
{                                                                                               \
    if (R3D.shader.shader_name.uniform.val != value) {                                          \
        R3D.shader.shader_name.uniform.val = value;                                             \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_FLOAT, 1                                                          \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_vec2(shader_name, uniform, value)                                        \
{                                                                                               \
    if (!Vector2Equals(R3D.shader.shader_name.uniform.val, value)) {                            \
        R3D.shader.shader_name.uniform.val = value;                                             \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_VEC2, 1                                                           \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_vec3(shader_name, uniform, value)                                        \
{                                                                                               \
    if (!Vector3Equals(R3D.shader.shader_name.uniform.val, value)) {                            \
        R3D.shader.shader_name.uniform.val = value;                                             \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_VEC3, 1                                                           \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_vec4(shader_name, uniform, value)                                        \
{                                                                                               \
    if (!Vector4Equals(R3D.shader.shader_name.uniform.val, value)) {                            \
        R3D.shader.shader_name.uniform.val = value;                                             \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_VEC4, 1                                                           \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_col3(shader_name, uniform, value)                                        \
{                                                                                               \
    Vector3 v = { value.r / 255.0f, value.g / 255.0f, value.b / 255.0f };                       \
    if (!Vector3Equals(R3D.shader.shader_name.uniform.val, v)) {                                \
        R3D.shader.shader_name.uniform.val = v;                                                 \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_VEC3, 1                                                           \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_col4(shader_name, uniform, value)                                        \
{                                                                                               \
    Vector4 v = { value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f };     \
    if (!Vector4Equals(R3D.shader.shader_name.uniform.val, v)) {                                \
        R3D.shader.shader_name.uniform.val = v;                                                 \
        rlSetUniform(                                                                           \
            R3D.shader.shader_name.uniform.loc,                                                 \
            &R3D.shader.shader_name.uniform.val,                                                \
            RL_SHADER_UNIFORM_VEC4, 1                                                           \
        );                                                                                      \
    }                                                                                           \
}

#define r3d_shader_set_mat4(shader_name, uniform, value)                                        \
{                                                                                               \
    rlSetUniformMatrix(R3D.shader.shader_name.uniform.loc, value);                              \
}


/* === Texture helper macros === */

#define r3d_texture_bind_2D(slot, texId)                    \
{                                                           \
    rlActiveTextureSlot(slot);                              \
    rlEnableTexture(texId);                                 \
}

#define r3d_texture_bind_opt_2D(slot, texId, altTex)        \
{                                                           \
    rlActiveTextureSlot(slot);                              \
    if (texId != 0) rlEnableTexture(texId);                 \
    else rlEnableTexture(R3D.texture.altTex);               \
}

#define r3d_texture_unbind_2D(slot)                         \
{                                                           \
    rlActiveTextureSlot(slot);                              \
    rlDisableTexture();                                     \
}

#define r3d_texture_bind_cubemap(slot, texId)               \
{                                                           \
    rlActiveTextureSlot(slot);                              \
    rlEnableTextureCubemap(texId);                          \
}

#define r3d_texture_unbind_cubemap(slot)                    \
{                                                           \
    rlActiveTextureSlot(slot);                              \
    rlDisableTextureCubemap();                              \
}


/* === Primitive helper macros */

#define r3d_primitive_draw_quad()                           \
{                                                           \
    r3d_primitive_draw(&R3D.primitive.quad);                \
}

#define r3d_primitive_draw_cube()                           \
{                                                           \
    r3d_primitive_draw(&R3D.primitive.cube);                \
}

#endif // R3D_STATE_H
